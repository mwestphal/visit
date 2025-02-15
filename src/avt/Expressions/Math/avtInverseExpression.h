// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtInverseExpression.h                       //
// ************************************************************************* //

#ifndef AVT_INVERSE_FILTER_H
#define AVT_INVERSE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtInverseExpression
//
//  Purpose:
//      A filter that calculates the inverse of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
// 
//    Justin Privitera, Mon Oct 28 10:15:57 PDT 2024
//    Pass in_ds to DoOperation().
//
// ****************************************************************************

class EXPRESSION_API avtInverseExpression : public avtUnaryMathExpression
{
  public:
                              avtInverseExpression();
    virtual                  ~avtInverseExpression();

    virtual const char       *GetType(void)  
                                         { return "avtInverseExpression"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating inverse"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


