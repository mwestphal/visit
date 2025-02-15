// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVariableByNodeQuery.C                         //
// ************************************************************************* //

#include <avtVariableByNodeQuery.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <float.h>
#include <PickVarInfo.h>
#include <DebugStream.h>
#include <QueryArgumentException.h>


using std::string;



// ****************************************************************************
//  Method: avtVariableByNodeQuery::avtVariableByNodeQuery
//
//  Purpose:
//      Construct an avtVariableByNodeQuery object.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 29, 2004
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 10:19:29 PDT 2011
//    Added domain, node.
//
// ****************************************************************************

avtVariableByNodeQuery::avtVariableByNodeQuery()
{
    domain = 0;
    node = 0;
    useGlobalId = 0;
}

// ****************************************************************************
//  Method: avtVariableByNodeQuery::~avtVariableByNodeQuery
//
//  Purpose:
//      Destruct an avtVariableByNodeQuery object.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtVariableByNodeQuery::~avtVariableByNodeQuery()
{
}


// ****************************************************************************
//  Method: avtVariableByNodeQuery::SetInputParams
//
//  Purpose:  Allows this query to read input parameters set by user.
//
//  Arguments:
//    params:     MapNode containing input.
//
//  Programmer:   Kathleen Biagas
//  Creation:     June 20, 2011
//
//  Modifications:
//    Kathleen Biagas, Thu Jan 10 08:12:47 PST 2013
//    Use newer MapNode methods that check for numeric entries and retrieves
//    to specific type.  Add error checking for size of passed vectors.
//
// ****************************************************************************

void
avtVariableByNodeQuery::SetInputParams(const MapNode &params)
{
    if (params.HasEntry("vars"))
    {
        stringVector v = params.GetEntry("vars")->AsStringVector();
        if (v.empty())
            EXCEPTION2(QueryArgumentException, "vars", 1);
        timeCurveSpecs["nResultsToStore"] = (int) v.size();
        pickAtts.SetVariables(v);
    }
    else
        EXCEPTION1(QueryArgumentException, "vars");

    if (params.HasNumericEntry("domain"))
        domain = params.GetEntry("domain")->ToInt();

    if (params.HasNumericEntry("element"))
        node = params.GetEntry("element")->ToInt();
    else
        EXCEPTION1(QueryArgumentException, "element");

    if (params.HasNumericEntry("use_global_id"))
        useGlobalId = params.GetEntry("use_global_id")->ToBool();
}


// ****************************************************************************
//  Method: avtVariableByNodeQuery::Preparation
//
//  Purpose:
//    Sets pickAtts information from queryAtts.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 29, 2004
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDataAttributes arg.
//
//    Kathleen Biagas, Tue Jun 21 10:20:37 PDT 2011
//    Domain, node and vars retrieved in SetInputParams.
//
// ****************************************************************************

void
avtVariableByNodeQuery::Preparation(const avtDataAttributes &inAtts)
{
    avtDataRequest_p dataRequest =
        GetInput()->GetOriginatingSource()->GetFullDataRequest();

    pickAtts.SetTimeStep(queryAtts.GetTimeStep());
    pickAtts.SetActiveVariable(dataRequest->GetVariable());
    pickAtts.SetDomain(domain);
    pickAtts.SetElementNumber(node);
    pickAtts.SetPickType(PickAttributes::DomainNode);
    pickAtts.SetElementIsGlobal(useGlobalId);

    avtPickByNodeQuery::Preparation(inAtts);
}


// ****************************************************************************
//  Method: avtVariableByNodeQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 29, 2004
//
//  Modifications:
//    Brad Whitlock, Tue Mar 13 11:26:59 PDT 2007
//    Updated due to code generation changes.
//
//    Kathleen Bonnell, Thu Nov 29 11:38:02 PST 2007
//    Ensure magnitude of vectors/tensors gets reported as the result, instead
//    of the first component.  Also ensure a failed query gets reported.
//
//    Brad Whitlock, Mon Oct 20 16:13:30 PDT 2008
//    Check to see if there are varInfo's in the pick attributes. There are
//    none in the case that the user accidentally performed the query on
//    a Mesh plot.
//
//    Kathleen Bonnell, Thu Feb  3 16:17:37 PST 2011
//    Verify that pickvarinfo's values aren't empty before attempting to
//    reference an element.
//
//    Kathleen Bonnell, Tue Mar  1 16:06:04 PST 2011
//    Allow output for multiple vars.
//
//    Kathleen Biagas, Tue Jun 21 10:20:37 PDT 2011
//    Domain and node retrieved in SetInputParams.
//
// ****************************************************************************

void
avtVariableByNodeQuery::PostExecute(void)
{
    avtPickQuery::PostExecute();

    if (PAR_Rank() == 0)
    {
        doubleVector vals;
        if (pickAtts.GetFulfilled())
        {
            // Special indication that the pick point should not be displayed.
            double cp[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            string msg;
            pickAtts.SetCellPoint(cp);
            pickAtts.CreateOutputString(msg);
            SetResultMessage(msg.c_str());
            if (pickAtts.GetNumVarInfos() == 1)
            {
                doubleVector dvals = pickAtts.GetVarInfo(0).GetValues();
                if (dvals.size() > 0)
                {
                    SetResultValue(dvals[dvals.size()-1]);
                }
                else
                {
                    debug3 << "Variable by Node query reports a fulfilled "
                           << "pick and has varInfo's, but the values "
                           << "vector is empty. This could happen when "
                           << "picking on a Material." << endl;
                    SetResultValues(vals);
                }
            }
            else if(pickAtts.GetNumVarInfos() > 1)
            {
                doubleVector dvals;
                for (int i = 0; i < pickAtts.GetNumVarInfos(); ++i)
                {
                    if (pickAtts.GetVarInfo(i).GetValues().size() > 0)
                    {
                       dvals.push_back(pickAtts.GetVarInfo(i).GetValues()[0]);
                    }
                }
                SetResultValues(dvals);
            }
            else
            {
                SetResultValues(vals);
            }
        }
        else
        {
            char msg[120];
            snprintf(msg, 120, "Could not retrieve information from domain "
                     " %d element %d.", domain, node);
            SetResultMessage(msg);
            SetResultValues(vals);
        }
    }
    pickAtts.PrepareForNewPick();
}


// ****************************************************************************
//  Method: avtVariableByNodeQuery::GetTimeCurveSpecs
//
//  Purpose:
//    Override default TimeCurveSpecs
//
//  Programmer:  Kathleen Bonnell
//  Creation:    July 8, 2008
//
//  Modifications:
//    Kathleen Biagas, Wed Sep 11, 2024
//    Added QueryAttributes argument (currently unused here).
//
// ****************************************************************************

const MapNode&
avtVariableByNodeQuery::GetTimeCurveSpecs(const QueryAttributes *)
{
    timeCurveSpecs["useVarForYAxis"] = true;
    return timeCurveSpecs;
}
