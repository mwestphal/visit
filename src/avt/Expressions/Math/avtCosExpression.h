// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtCosExpression.h                          //
// ************************************************************************* //

#ifndef AVT_COS_FILTER_H
#define AVT_COS_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtCosExpression
//
//  Purpose:
//      A filter that calculates the sin of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
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

class EXPRESSION_API avtCosExpression : public avtUnaryMathExpression
{
  public:
                              avtCosExpression();
    virtual                  ~avtCosExpression();

    virtual const char       *GetType(void)   { return "avtCosExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating cosine"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


