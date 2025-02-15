// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtConservativeSmoothingExpression.C                   //
// ************************************************************************* //

#include <avtConservativeSmoothingExpression.h>

#include <algorithm>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


using std::sort;


// ****************************************************************************
//  Method: avtConservativeSmoothingExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2005
//
//  Modifications:
//
//  Alister Maguire, Thu Jun 18 10:02:58 PDT 2020
//  Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtConservativeSmoothingExpression::avtConservativeSmoothingExpression()
{
    haveIssuedWarning = false;
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtConservativeSmoothingExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2005
//
// ****************************************************************************

avtConservativeSmoothingExpression::~avtConservativeSmoothingExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConservativeSmoothingExpression::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2005
//
// ****************************************************************************

void
avtConservativeSmoothingExpression::PreExecute(void)
{
    avtUnaryMathExpression::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtConservativeSmoothingExpression::DoOperation
//
//  Purpose:
//      Calculates the conservative smoothing of the input.
//
//  Arguments:
//      in1           The first input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//      in_ds         The input dataset.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2005
//
//  Modifications:
//
//    Hank Childs, Tue Aug 23 09:37:51 PDT 2005
//    Fix indexing bug.
//
//    Hank Childs, Fri Jun  9 14:34:50 PDT 2006
//    Remove unused variable.
//
//    Kathleen Biagas, Wed Apr 4 12:34:10 PDT 2012
//    Use double instead of float.
// 
//    Justin Privitera, Mon Oct 28 10:15:57 PDT 2024
//    Pass in the input dataset.
//
// ****************************************************************************

void
avtConservativeSmoothingExpression::DoOperation(vtkDataArray *in1, 
    vtkDataArray *out, int ncomponents,int ntuples, vtkDataSet *in_ds)
{
    if (cur_mesh->GetDataObjectType() != VTK_RECTILINEAR_GRID &&
        cur_mesh->GetDataObjectType() != VTK_STRUCTURED_GRID)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The mean filter expression only "
                                      "operates on structured grids.");
            haveIssuedWarning = true;
        }
        return;
    }

    int dims[3];
    if (cur_mesh->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) cur_mesh;
        rgrid->GetDimensions(dims);
    }
    else
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) cur_mesh;
        sgrid->GetDimensions(dims);
    }

    bool nodeCentered = true;
    if (dims[0]*dims[1]*dims[2] != ntuples)
        nodeCentered = false;

    if (!nodeCentered)
    {
        dims[0] -= 1;
        dims[1] -= 1;
        dims[2] -= 1;
    }

    if (dims[2] <= 1)
    {
        for (int i = 0 ; i < dims[0] ; i++)
        {
            for (int j = 0 ; j < dims[1] ; j++)
            {
                int idx = j*dims[0]+i;
                double max = -DBL_MAX;
                double min = +DBL_MAX;
                for (int ii = i-1 ; ii <= i+1 ; ii++)
                {
                    if (ii < 0 || ii >= dims[0])
                        continue;
                    for (int jj = j-1 ; jj <= j+1 ; jj++)
                    {
                        if (jj < 0 || jj >= dims[1])
                            continue;
                        if (ii == i && jj == j)
                            continue;
                        int idx2 = jj*dims[0] + ii;
                        double val = in1->GetTuple1(idx2);
                        max = (val > max ? val : max);
                        min = (val < min ? val : min);
                    }
                }
                double val = in1->GetTuple1(idx);
                val = (val > max ? max : val);
                val = (val < min ? min : val);
                out->SetTuple1(idx, val);
            }
        }
    }
    else
    {
        for (int i = 0 ; i < dims[0] ; i++)
        {
            for (int j = 0 ; j < dims[1] ; j++)
            {
                for (int k = 0 ; k < dims[2] ; k++)
                {
                    int idx = k*dims[0]*dims[1] + j*dims[0]+i;
                    double max = -DBL_MAX;
                    double min = +DBL_MAX;
                    for (int ii = i-1 ; ii <= i+1 ; ii++)
                    {
                        if (ii < 0 || ii >= dims[0])
                            continue;
                        for (int jj = j-1 ; jj <= j+1 ; jj++)
                        {
                            if (jj < 0 || jj >= dims[1])
                                continue;
                            for (int kk = k-1 ; kk <= k+1 ; kk++)
                            {
                                if (kk < 0 || kk >= dims[2])
                                    continue;
                                if (ii == i && jj == j && kk == k)
                                    continue;
                                int idx2 = kk*dims[1]*dims[0] + jj*dims[0] +ii;
                                double val = in1->GetTuple1(idx2);
                                max = (val > max ? val : max);
                                min = (val < min ? val : min);
                            }
                        }
                    }
                    double val = in1->GetTuple1(idx);
                    val = (val > max ? max : val);
                    val = (val < min ? min : val);
                    out->SetTuple1(idx, val);
                }
            }
        }
    }
}


