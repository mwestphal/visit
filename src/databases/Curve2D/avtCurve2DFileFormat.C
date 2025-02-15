// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCurve2DFileFormat.C                         //
// ************************************************************************* //

#include <avtCurve2DFileFormat.h>

#include <vector>
#include <string>

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <DBOptionsAttributes.h>
#include <StringHelpers.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include "visit_gzstream.h"

#include <errno.h>
#include <float.h>
#include <stdlib.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtCurve2DFileFormat constructor
//
//  Arguments:
//      fname    The name of the curve file.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Do not do so much work in the constructor.
//
//    Kathleen Bonnell, Fri Oct 28 13:02:51 PDT 2005
//    Added curveTime, curveCycle.
//
//    Kathleen Biagas, Fri Aug 31 14:22:49 PDT 2018
//    Added read options (currently unused).
//
// ****************************************************************************

avtCurve2DFileFormat::avtCurve2DFileFormat(const char *fname, const DBOptionsAttributes *)
    : avtSTSDFileFormat(fname)
{
    filename = fname;
    readFile = false;
    curveTime = INVALID_TIME;
    curveCycle = INVALID_CYCLE;
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
// ****************************************************************************

avtCurve2DFileFormat::~avtCurve2DFileFormat()
{
    for (size_t i = 0 ; i < curves.size() ; i++)
    {
        curves[i]->Delete();
    }
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetMesh
//
//  Purpose:
//      Returns the curve associated with a curve name.
//
//  Arguments:
//      name       The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Jan 15 10:54:14 PST 2003
//    Increment the reference count for the curve, since we own the memory for
//    it and the calling function believes it also owns it.
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Make sure we have read in the file first.
//
//    Hank Childs, Fri Aug  1 21:19:28 PDT 2003
//    Retro-fit for STSD.
//
// ****************************************************************************

vtkDataSet *
avtCurve2DFileFormat::GetMesh(const char *name)
{
    if (!readFile)
    {
        ReadFile();
    }

    for (size_t i = 0 ; i < curves.size() ; i++)
    {
        if (strcmp(curveNames[i].c_str(), name) == 0)
        {
            //
            // The calling function will think it owns the return mesh, so
            // increment its reference count.
            //
            curves[i]->Register(NULL);
            return curves[i];
        }
    }

    EXCEPTION1(InvalidVariableException, name);
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetVar
//
//  Purpose:
//      Returns a variable.
//
//  Notes:      This is not meaningful for this file type and is here only to
//              meet the base type's interface.
//
//  Arguments:
//      <unused>  The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 21:19:28 PDT 2003
//    Retro-fit for STSD.
//
// ****************************************************************************

vtkDataArray *
avtCurve2DFileFormat::GetVar(const char *name)
{
    EXCEPTION1(InvalidVariableException, name);
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this curve file.  There is only one
//      mesh, the curve.  Each curve gets its own domain,for easy subselection.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Make sure we have read in the file first.
//
//    Hank Childs, Fri Aug  1 21:01:51 PDT 2003
//    Mark curves as "curve" type.
//
//    Kathleen Bonnell, Thu Aug  3 08:42:33 PDT 2006 
//    Added DataExtents to CurveMetaData. 
//
//    Kathleen Bonnell, Tue Jan 20 11:04:33 PST 2009
//    Added SpatialExtents to CurveMetaData. 
//
//    Brad Whitlock, Mon Jul 23 12:07:07 PDT 2012
//    Iterate based on curveNames since curves will not be populated on mdserver.
//
// ****************************************************************************

void
avtCurve2DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!readFile)
    {
        ReadFile();
    }

    for (size_t i = 0 ; i < curveNames.size() ; i++)
    {
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = curveNames[i];
        curve->hasSpatialExtents = true;
        curve->minSpatialExtents = spatialExtents[i*2];
        curve->maxSpatialExtents = spatialExtents[i*2+1];
        curve->hasDataExtents = true;
        curve->minDataExtents = dataExtents[i*2];
        curve->maxDataExtents = dataExtents[i*2+1];
        md->Add(curve);
    }
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::ReadFile
//
//  Purpose:
//      Actually reads in from a file.  This is pretty dependent on formats
//      that have one point per line.  When there are runs of points, followed
//      by non-points, that is assumed to be a new line.
//
//  Arguments:
//      ifile   The file to read in.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Jan 15 13:44:39 PST 2003
//    Fix parsing error where whitespace was getting prepended to the curve
//    names.
//
//    Hank Childs, Thu Apr  3 16:58:33 PST 2003
//    It is now acceptable to have whitespace between the curves to indicate
//    a break in the curve.  Add parsing for this.
//
//    Hank Childs, Fri Apr  4 09:04:17 PST 2003
//    Fixed bug from yesterday where wrong string is being sent into GetPoint.
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Do some of the work formerly done by the constructor.
//
//    Hank Childs, Thu Sep 23 14:54:07 PDT 2004
//    Add support for files with extra whitespace and files that have no 
//    headers.
//
//    Kathleen Bonnell, Fri Oct 28 13:02:51 PDT 2005
//    Parse 'TIME' and 'CYCLE' from headers not followed by data values. 
//
//    Kathleen Bonnell, Mon Jul 31 10:15:00 PDT 2006 
//    Represent curve as 1D RectilinearGrid instead of PolyData. 
//
//    Kathleen Bonnell, Thu Aug  3 08:42:33 PDT 2006 
//    Save DataExtents. 
//
//    Mark C. Miller, Tue Oct 31 20:33:29 PST 2006
//    Replaced Exception on INVALID_POINT with warning macro.
//    Added VALID_XVALUE for case where X but no Y is given at the END of
//    a curve specification and interpreted it as a "zone-centered" curve.
//    Added logic to re-center curve for the "zone-centered" case
//
//    Mark C. Miller, Wed Nov 15 16:24:06 PST 2006
//    Deal with possible exception during INVALID_POINT_WARNING. Fix
//    lack of suppression of more than 5 lines of errors. Fix possible
//    reference to array index -1 when xl.size() or yl.size()==0
//
//    Hank Childs, Mon Dec  8 13:51:16 PST 2008
//    For files with multiple curves, we were incorrectly throwing away
//    the first point of curve N+1 if it was identical to the last point
//    of curve N.
//
//    Kathleen Bonnell, Tue Jan 20 11:12:54 PST 2009
//    Add spatial extents.
//
//    Jeremy Meredith, Fri Jan  8 16:32:25 EST 2010
//    Made warning be invalid file exception in strict mode.
//
//    Brad Whitlock, Mon Jul 23 12:06:38 PDT 2012
//    Build lightweight rectilinear grids on mdserver.
//
//    Kathleen Biagas, Tue Jul 15 14:16:46 MST 2014
//    Change from using float to double.
//
//    Kathleen Biagas, Fri Aug 31 14:23:59 PDT 2018
//    Support matlab-sytle comments ("%").
//
//    Justin Privitera, Tue Jul  5 14:40:55 PDT 2022
//    Changed 'supressed' to 'suppressed'.
// ****************************************************************************

#define INVALID_POINT_WARNING(X)                                        \
{                                                                       \
    if (invalidPointCount++ < 6)                                        \
    {                                                                   \
        char msg[512] = "Further warnings will be suppressed";           \
        if (invalidPointCount < 6)                                      \
        {                                                               \
            snprintf(msg, sizeof(msg),"Encountered invalid point "      \
                "at or near line %d beginning with \"%s\"",             \
                lineCount, lineName.c_str());                           \
        }                                                               \
        TRY                                                             \
        {                                                               \
            if (!avtCallback::IssueWarning(msg))                        \
                cerr << msg << endl;                                    \
        }                                                               \
        CATCH(VisItException)                                           \
        {                                                               \
            cerr << msg << endl;                                        \
        }                                                               \
        ENDTRY                                                          \
    }                                                                   \
    xl.push_back(X);                                                    \
    if (yl.size())                                                      \
        yl.push_back(yl[yl.size()-1]);                                  \
    else                                                                \
        yl.push_back(X);                                                \
    breakpoint_following.push_back(false);                              \
}

void
avtCurve2DFileFormat::ReadFile(void)
{
    int invalidPointCount = 0;
    int lineCount = 1;
    visit_ifstream ifile(filename.c_str());

    if (ifile().fail())
    {
        debug1 << "Unable to open file " << filename.c_str() << endl;
        return;
    }

    //
    // Read in all of the points and store where there are lines between them
    // so we can re-construct them later.
    //
    vector<double> xl;
    vector<double> yl;
    vector<bool>  breakpoint_following;
    vector<int>   cutoff;
    vector<avtCentering> centering;
    avtCentering useCentering = AVT_NODECENT;
    string  headerName = "";
    curveTime = INVALID_TIME;
    curveCycle = INVALID_CYCLE;
    CurveToken lastt = VALID_POINT;
    bool justStartedNewCurve = false;
    xl.reserve(1000);
    yl.reserve(1000);
    while (!ifile().eof())
    {
        double   x, y;
        string  lineName;
        CurveToken t = GetPoint(ifile(), x, y, lineName);
        switch (t)
        {
          case VALID_POINT:
          {
            if (headerName.find_first_not_of("#%") != string::npos)
            {
                string str1 = 
                          headerName.substr(headerName.find_first_not_of("#%"));
                string str2 = str1.substr(str1.find_first_not_of(" \t"));
                curveNames.push_back(str2);
                headerName = "";
            }
            size_t len = xl.size();
            bool shouldAddPoint = true;
            if (len > 0)
            {
                if (x == xl[len-1] && y == yl[len-1])
                {
                    if (justStartedNewCurve == false)
                        shouldAddPoint = false;
                }
            }
            if (shouldAddPoint)
            {
                xl.push_back(x);
                yl.push_back(y);
                breakpoint_following.push_back(false);
                useCentering = AVT_NODECENT;
                justStartedNewCurve = false;
            }
            break;
          }
          case HEADER:
          {
            if (headerName != "")
            {
                // If we parsed a header followed by another header,
                // see if it has TIME. 
                size_t timePos = headerName.find("TIME");
                if ( timePos != string::npos)
                {
                    string tStr = headerName.substr(timePos+4);
                    char *endstr = NULL;
                    curveTime = strtod(tStr.c_str(), &endstr);
                    if (strcmp(endstr, tStr.c_str()) == 0)
                        curveTime = INVALID_TIME;
                }
                else
                {
                    size_t cyclePos = headerName.find("CYCLE");
                    if ( cyclePos != string::npos)
                    {
                        string cyStr = headerName.substr(cyclePos+5);
                        char *endstr = NULL;
                        curveCycle = (int)strtod(cyStr.c_str(), &endstr);
                        if (strcmp(endstr, cyStr.c_str()) == 0)
                            curveCycle = INVALID_CYCLE;
                    }
                }
            }
  
            if (lineName.find_first_not_of("#%") != string::npos)
            {
                headerName = lineName;
            }
            centering.push_back(useCentering);
            cutoff.push_back((int)xl.size());
            justStartedNewCurve = true;
            break;
          }
          case WHITESPACE:
          {
            if (breakpoint_following.size() > 0)
                breakpoint_following[breakpoint_following.size()-1] = true;
            break;
          }
          case INVALID_POINT:
          {
              if (this->GetStrictMode())
                  EXCEPTION2(InvalidFilesException, GetFilename(),
                             "Found a bad point");
              if (xl.size())
              {
                  INVALID_POINT_WARNING(xl[xl.size()-1]);
              }
              else
              {
                  INVALID_POINT_WARNING(0);
              }
              break;
          }
          case VALID_XVALUE:
          {
              if (lastt == VALID_XVALUE)
              {
                  if (this->GetStrictMode())
                      EXCEPTION2(InvalidFilesException, GetFilename(),
                                 "Found a bad point");
                  INVALID_POINT_WARNING(x);
                  useCentering = AVT_NODECENT;
              }
              else
              {
                  xl.push_back(x);
                  yl.push_back(yl[yl.size()-1]);
                  breakpoint_following.push_back(false);
                  useCentering = AVT_ZONECENT;
              }
              break;
          }
       }
       lastt = t;
       //lastx = x;
       lineCount++;
    }  

    // If we parsed a header not followed by data values, see if
    // it is TIME. 
    if (headerName != "")
    {
        size_t timePos = headerName.find("TIME");
        if ( timePos != string::npos)
        {
            string tStr = headerName.substr(timePos+4);
            char *endstr = NULL;
            curveTime = strtod(tStr.c_str(), &endstr);
            if (strcmp(endstr, tStr.c_str()) == 0)
                curveTime = INVALID_TIME;
        }
        else
        {
            size_t cyclePos = headerName.find("CYCLE");
            if ( cyclePos != string::npos)
            {
                string cyStr = headerName.substr(cyclePos+5);
                char *endstr = NULL;
                curveCycle = (int)strtod(cyStr.c_str(), &endstr);
                if (strcmp(endstr, cyStr.c_str()) == 0)
                    curveCycle = INVALID_CYCLE;
            }
        }
    }

    //
    // Now we can construct the curve as vtkPolyData.
    //
    int start = 0;
    cutoff.push_back((int)xl.size());       // Make logic easier.
    centering.push_back(useCentering); //      ditto
    int curveIndex = 0; (void) curveIndex;
    for (size_t i = 0 ; i < cutoff.size() ; i++)
    {
        if (start == cutoff[i])
        {
            continue;
        }
       
        //
        // Add all of the points to an array.
        //
        int nPts = cutoff[i] - start - (centering[i] == AVT_NODECENT ? 0 : 1);
#ifdef MDSERVER
        vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
#else
        vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nPts,VTK_DOUBLE);
 
        vtkDoubleArray    *vals = vtkDoubleArray::New();
        vals->SetNumberOfComponents(1);
        vals->SetNumberOfTuples(nPts);
        if (curveNames.size() != 0) 
            vals->SetName(curveNames[curveIndex++].c_str());
        else 
            vals->SetName("curve");

        rg->GetPointData()->SetScalars(vals);
        vals->Delete();
        vtkDoubleArray *xc = vtkDoubleArray::SafeDownCast(rg->GetXCoordinates());
#endif

        double dmin = DBL_MAX;
        double dmax = -DBL_MAX;
        double smin = DBL_MAX;
        double smax = -DBL_MAX;
        for (int j = 0 ; j < nPts ; j++)
        {
            double X, Y;
            if (centering[i] == AVT_NODECENT)
                X = xl[start+j];
            else
                X = (xl[start+j]+xl[start+j+1])/2.0;
            Y = yl[start+j];
#ifndef MDSERVER
            xc->SetValue(j, X);
            vals->SetValue(j, Y);
#endif
            if (Y < dmin)
                dmin = Y;
            if (Y > dmax)
                dmax = Y;
            if (X < smin)
                smin = X;
            if (X > smax)
                smax = X;
        }

        curves.push_back(rg);
        spatialExtents.push_back(smin);
        spatialExtents.push_back(smax);
        dataExtents.push_back(dmin);
        dataExtents.push_back(dmax);

        //
        // Set ourselves up for the next iteration.
        //
        start = cutoff[i];
    }

    //
    // It is possible to have a file that has no header.  This should be
    // interpreted as one curve.  Check for this, since we will later assume
    // the number of curves and the number of curve names is the same.
    //
    if (curves.size() == 1 && curveNames.size() == 0)
    {
        curveNames.push_back("curve");
    }

    if (curves.size() != curveNames.size())
    {
        debug1 << "The number of curves does not match the number of curve "
               << "names.  Cannot continue with this file." << endl;
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    readFile = true;
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetPoint
//
//  Purpose:
//      Gets a point from a line.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Apr  3 16:58:33 PST 2003
//    Identify different return types to make parsing easier.
//
//    Hank Childs, Mon May 24 14:45:14 PDT 2004
//    Treat the line "end" as white space, since it screws up our parsing
//    and is not really necessary for us.  Also treat parenthesis as square
//    brackets, since parenthesis are special for us.
//
//    Brad Whitlock, Tue Jun 29 11:50:41 PDT 2004
//    Fixed for Windows compiler.
//
//    Hank Childs, Thu Sep 23 14:54:07 PDT 2004
//    Add support for Fortran-style scientific notation (5.05D-2).
//
//    Hank Childs, Fri Jul 29 14:34:39 PDT 2005
//    Add support for tabs.
//
//    Mark C. Miller, Tue Oct 31 20:33:29 PST 2006
//    Added code to detect more closely possible errors from strtod.
//    Added logic to handle VALID_XVALUE case.
//
//    Mark C. Miller, Wed Nov 15 16:24:06 PST 2006
//    Reset errno before calling strtod
//
//    Jeremy Meredith, Thu Jan  7 12:11:29 EST 2010
//    Make sure read data is ASCII.
//
//    Jeremy Meredith, Fri Jan  8 16:32:40 EST 2010
//    Only to ASCII check in strict mode since it's basically the whole file.
//
//    Kathleen Biagas, Tue Jul 15 14:16:46 MST 2014
//    Change from using float to double.
//
//    Kathleen Biagas, Fri Aug 31 14:23:59 PDT 2018
//    Support matlab-sytle comments ("%").
//
// ****************************************************************************

CurveToken
avtCurve2DFileFormat::GetPoint(istream &ifile, double &x, double &y, string &ln)
{
    char line[256];
    ifile.getline(line, 256, '\n');

    // Do an ASCII check.  We only support text files.
    if (GetStrictMode() && !StringHelpers::IsPureASCII(line,256))
        EXCEPTION2(InvalidFilesException, filename, "Not ASCII.");

    //
    // Parenthesis are special characters for variables names, etc, so just
    // change them to square brackets to "go with the flow"...
    //
    size_t i, nchars = strlen(line);
    for (i = 0 ; i < nchars ; i++)
    {
        if (line[i] == '(')
            line[i] = '<';
        else if (line[i] == ')')
            line[i] = '>';
    }
    ln = line;

    //
    // Pick out some of the harder to parse cases.
    //
    if (strstr(line, "#") != NULL)
    {
        return HEADER;
    }
    if (strstr(line, "%") != NULL)
    {
        return HEADER;
    }
    bool allSpace = true;
    size_t len = strlen(line);
    for (i = 0 ; i < len ; i++)
    {
        if (!isspace(line[i]))
        {
            allSpace = false;
        }
    }
    if (allSpace)
    {
        return WHITESPACE;
    }
    if (strncmp(line, "end", strlen("end")) == 0)
    {
        // We will infer that we have hit the end when we find a new token.
        // Just treat this as white space to make our parsing rules easier.
        return WHITESPACE;
    }

    //
    // We are assuming that we a number.  Fortran-style scientific notation
    // uses 'D' when we are used to seeing 'E'.  So just switch them out.
    //
    for (i = 0 ; i < len ; i++)
    {
        if (line[i] == 'D' || line[i] == 'd')
            line[i] = 'E';
        if (line[i] == '\t')
            line[i] = ' ';
    }

    char *ystr = NULL;

    errno = 0;
    x = strtod(line, &ystr);
    if (((x == 0.0) && (ystr == line)) || (errno == ERANGE))
    {
        return INVALID_POINT;
    }
    if (ystr == NULL)
    {
        return VALID_XVALUE;
    }
    ystr = strstr(ystr, " ");
    if (ystr == NULL || ystr == line)
    {
        return VALID_XVALUE;
    }
    
    // Get past the space.
    ystr++;

    char *tmpstr;
    errno = 0;
    y = strtod(ystr, &tmpstr);
    if (((y == 0.0) && (tmpstr == ystr)) || (errno == ERANGE))
    {
        return INVALID_POINT;
    }

    ln = "";
    return VALID_POINT;
}    


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetTime
//
//  Purpose: Return the time associated with this curve file
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 28, 2005 
//
// ****************************************************************************

double 
avtCurve2DFileFormat::GetTime(void)
{
    return curveTime;
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetCycle
//
//  Purpose: Return the cycle associated with this curve file
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 28, 2005 
//
// ****************************************************************************

int 
avtCurve2DFileFormat::GetCycle(void)
{
    return curveCycle;
}

