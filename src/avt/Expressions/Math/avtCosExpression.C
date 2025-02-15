// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtCosExpression.C                          //
// ************************************************************************* //

#include <avtCosExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtCosExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCosExpression::avtCosExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCosExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCosExpression::~avtCosExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCosExpression::DoOperation
//
//  Purpose:
//      Performs the cosine to each component, tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//      in_ds         The input dataset.
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 15, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:25:26 PST 2002
//    Added support for vectors and arbitrary data types.
// 
//    Justin Privitera, Mon Oct 28 10:15:57 PDT 2024
//    Pass in the input dataset.
//
// ****************************************************************************
 
void
avtCosExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            double val = in->GetComponent(i, j);
            out->SetComponent(i, j, cos(val));
        }
    }
}


