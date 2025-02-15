// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France

//const static char * VTK_C_Q_S__C_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";

#include "vtkCQS.h"

#include <vtkMath.h>
#include <vtkTriangle.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkCell.h>
#include <vtkCell3D.h>
#include <vtkFieldData.h>

vtkStandardNewMacro(vtkCQS);

vtkCQS::vtkCQS()
{
}

vtkCQS::~vtkCQS()
{
}

#define ADD_VEC(a,b) a[0]+=b[0];a[1]+=b[1];a[2]+=b[2]
#define SCALE_VEC(a,b) a[0]*=b;a[1]*=b;a[2]*=b
#define ZERO_VEC(a) a[0]=0;a[1]=0;a[2]=0
#define MAX_CELL_POINTS 128
#define MAX_FACE_POINTS 16

#define VTK_CQS_EPSILON 1e-12

static inline void TETRA_CQS_VECTOR( double v0[3], double v1[3], double v2[3], double p[3], double cqs[3] )
{
    double surface = fabs( vtkTriangle::TriangleArea (v0,v1,v2) );

    vtkTriangle::ComputeNormal( v0,v1,v2 , cqs );

    // inverse face normal if not toward opposite vertex
    double edge[3];
    edge[0] = p[0] - v0[0];
    edge[1] = p[1] - v0[1];
    edge[2] = p[2] - v0[2];
    if( vtkMath::Dot(edge,cqs) < 0 )
    {
        cqs[0] = - cqs[0];
        cqs[1] = - cqs[1];
        cqs[2] = - cqs[2];
    }

    SCALE_VEC( cqs , surface / 6.0 );
}

static inline void TRIANGLE_CQS_VECTOR( double v0[3], double v1[3], double p[3], double cqs[3] )
{
    double length = fabs( vtkMath::Distance2BetweenPoints(v0,v1) );
    double a[3], b[3], c[3];
    for(int i=0;i<3;i++)
    {
        a[i] = v1[i] - v0[i];
        b[i] = p[i] - v0[i];
    }
    vtkMath::Cross( a, b, c );
    vtkMath::Cross( c , a , cqs );
    double n = vtkMath::Norm( cqs );
    SCALE_VEC( cqs , length / (n*2.0) );
}

int vtkCQS::RequestData(vtkInformation *request,
                        vtkInformationVector **inputVector,
                        vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get connected input & output
    vtkDataSet* _output = vtkDataSet::SafeDownCast( outInfo->Get(vtkDataObject::DATA_OBJECT()) );
    vtkDataSet* _input = vtkDataSet::SafeDownCast( inInfo->Get(vtkDataObject::DATA_OBJECT()) );

    if( _input==0 || _output==0 )
    {
        vtkErrorMacro(<<"missing input/output connection\n");
        return 0;
    }

    // detect dimensionality
    /*
    int dimension = 3;
    {
        double bounds[6];
        _input->GetBounds(bounds);
        double minDim = bounds[1] - bounds[0];
        if( (bounds[3]-bounds[2]) < minDim ) minDim = bounds[3]-bounds[2];
        if( (bounds[5]-bounds[4]) < minDim ) minDim = bounds[5]-bounds[4];
        if( _input->IsA("vtkPolyData") || minDim==0.0 )
        {
            dimension = 2;
        }
    }
    */

    vtkIdType nCells = _input->GetNumberOfCells();
    vtkIdType nCellNodes = 0;
    for(vtkIdType i=0;i<nCells;i++)
    {
        nCellNodes += _input->GetCell(i)->GetNumberOfPoints();
    }

    _output->ShallowCopy(_input);

    vtkDoubleArray* cqs = vtkDoubleArray::New();
    cqs->SetName("CQS");
    cqs->SetNumberOfComponents(3);
    cqs->SetNumberOfTuples(nCellNodes);
    cqs->FillComponent(0, 0.0);
    cqs->FillComponent(1, 0.0);
    cqs->FillComponent(2, 0.0);

    vtkIdType curPoint = 0;
    for(vtkIdType c=0;c<nCells;c++)
    {
        vtkCell* cell = _input->GetCell(c);
        int np = cell->GetNumberOfPoints();

        double cellCenter[3] = {0,0,0};
        double cellPoints[MAX_CELL_POINTS][3];
        double cellVectors[MAX_CELL_POINTS][3];
        double tmp[3];

        for(int p=0;p<np;p++)
        {
            _input->GetPoint( cell->GetPointId(p), cellPoints[p] );
            ADD_VEC( cellCenter , cellPoints[p] );
            ZERO_VEC( cellVectors[p] );
        }
        SCALE_VEC(cellCenter,1.0/np);

        // -= 3 D =-
        if( cell->GetCellDimension() == 3 )
        {
            // JSM December 23, 2009: tet is 4 points, not 3
            if( np == 4 ) // cell is a tetrahedra
            {
                TETRA_CQS_VECTOR( cellPoints[0], cellPoints[1], cellPoints[2], cellPoints[3] , tmp );
                ADD_VEC(cellVectors[3],tmp);

                TETRA_CQS_VECTOR( cellPoints[1], cellPoints[2], cellPoints[3], cellPoints[0] , tmp );
                ADD_VEC(cellVectors[0],tmp);

                TETRA_CQS_VECTOR( cellPoints[2], cellPoints[3], cellPoints[0], cellPoints[1] , tmp );
                ADD_VEC(cellVectors[1],tmp);

                TETRA_CQS_VECTOR( cellPoints[3], cellPoints[0], cellPoints[1], cellPoints[2] , tmp );
                ADD_VEC(cellVectors[2],tmp);
            }
            else if( np > 4 )
            {
                vtkCell3D* cell3d = static_cast<vtkCell3D*>( cell );
                int nf = cell->GetNumberOfFaces();
                for(int f=0;f<nf;f++)
                {
                    const vtkIdType *faceIds = 0;
                    vtkIdType nfp = cell->GetFace(f)->GetNumberOfPoints();
                    cell3d->GetFacePoints(f,faceIds);
                    if( nfp == 3 ) // face is a triangle
                    {
                        TETRA_CQS_VECTOR( cellCenter, cellPoints[faceIds[0]], cellPoints[faceIds[1]], cellPoints[faceIds[2]] , tmp );
                        ADD_VEC(cellVectors[faceIds[2]],tmp);

                        TETRA_CQS_VECTOR( cellCenter, cellPoints[faceIds[1]], cellPoints[faceIds[2]], cellPoints[faceIds[0]] , tmp );
                        ADD_VEC(cellVectors[faceIds[0]],tmp);

                        TETRA_CQS_VECTOR( cellCenter, cellPoints[faceIds[2]], cellPoints[faceIds[0]], cellPoints[faceIds[1]] , tmp );
                        ADD_VEC(cellVectors[faceIds[1]],tmp);
                    }
                    else if( nfp > 3 ) // generic case
                    {
                        double faceCenter[3] = {0,0,0};
                        for(int p=0;p<nfp;p++)
                        {
                            ADD_VEC( faceCenter , cellPoints[faceIds[p]] );
                        }
                        SCALE_VEC( faceCenter, 1.0/nfp );
                        for(int p=0;p<nfp;p++)
                        {
                            int p2 = (p+1) % nfp ;
                            TETRA_CQS_VECTOR( cellCenter, faceCenter, cellPoints[faceIds[p]] , cellPoints[faceIds[p2]] , tmp );
                            ADD_VEC( cellVectors[faceIds[p2]] , tmp );

                            TETRA_CQS_VECTOR( cellCenter, faceCenter, cellPoints[faceIds[p2]] , cellPoints[faceIds[p]] , tmp );
                            ADD_VEC( cellVectors[faceIds[p]] , tmp );
                        }
                    }
                }
            }
        }

        // -= 2 D =-
        else
        {
            if( np == 3 ) // cell is a triangle
            {
                TRIANGLE_CQS_VECTOR( cellPoints[0] , cellPoints[1] , cellPoints[2] , tmp );
                ADD_VEC( cellVectors[2] , tmp );

                TRIANGLE_CQS_VECTOR( cellPoints[1] , cellPoints[2] , cellPoints[0] , tmp );
                ADD_VEC( cellVectors[0] , tmp );

                TRIANGLE_CQS_VECTOR( cellPoints[2] , cellPoints[0] , cellPoints[1] , tmp );
                ADD_VEC( cellVectors[1] , tmp );
            }
            else if( np > 3) // generic case
            {
                for(int f=0;f<np;f++)
                {
                    int e0 = f;
                    int e1 = (f+1)%np;
                    // JSM December 23, 2009: Added pixel support via
                    // fixup; clockwise ordering is assumed here
                    if (cell->GetCellType() == VTK_PIXEL)
                    {
                        if (e0==2) e0=3; else if (e0==3) e0=2;
                        if (e1==2) e1=3; else if (e1==3) e1=2;
                    }
                    double tmp[3];
                    TRIANGLE_CQS_VECTOR( cellCenter , cellPoints[e0] , cellPoints[e1] , tmp );
                    ADD_VEC( cellVectors[e1] , tmp );

                    TRIANGLE_CQS_VECTOR( cellCenter , cellPoints[e1] , cellPoints[e0] , tmp );
                    ADD_VEC( cellVectors[e0] , tmp );
                }
            }
        }

        // check cqs consistency
        double v[3] = {0,0,0};
        for(int p=0;p<np;p++)
        {
            ADD_VEC(v,cellVectors[p]);
            cqs->SetTuple( curPoint + p , cellVectors[p] );
        }
        if( vtkMath::Dot(v,v) > VTK_CQS_EPSILON )
        {
            printf("Bad CQS sum : %g\n",vtkMath::Dot(v,v));
        }

        curPoint += np;
    }

    _output->GetFieldData()->AddArray( cqs );
    cqs->Delete();

    return 1;
}

