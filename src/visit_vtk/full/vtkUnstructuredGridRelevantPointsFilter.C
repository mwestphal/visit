// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkUnstructuredGridRelevantPointsFilter.h"

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkCellData.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 17:10:03 PST 2002
//    Replace 'New' method with Macro to match VTK 4.0 API.
//
// ****************************************************************************

vtkStandardNewMacro(vtkUnstructuredGridRelevantPointsFilter);


// ****************************************************************************
//  Method: vtkUnstructuredGridRelevantPointsFilter::RequestData
//
//  Modifications:
//    Hank Childs, Sun Mar 13 14:12:38 PST 2005
//    Fix memory leak.
//
//    Hank Childs, Sun Mar 29 16:26:27 CDT 2009
//    Remove call to BuildLinks.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK 9: use vtkCellArrayIterator.
//
// ****************************************************************************
int
vtkUnstructuredGridRelevantPointsFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  vtkUnstructuredGrid  *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro(<<"Beginning UnstructuredGrid Relevant Points Filter ");

  if (input == NULL)
    {
    vtkErrorMacro(<<"Input is NULL");
    return 1;
    }

  vtkPoints    *inPts  = input->GetPoints();
  vtkIdType   numInPts = input->GetNumberOfPoints();
  vtkIdType   numCells = input->GetNumberOfCells();
  output->Allocate(numCells);

  if ( (numInPts<1) || (inPts == NULL ) )
    {
    vtkErrorMacro(<<"No data to Operate On!");
    return 1;
    }

  vtkIdType *pointMap = new vtkIdType[numInPts];
  for (vtkIdType i = 0 ; i < numInPts ; i++)
    {
    pointMap[i] = -1;
    }

  auto cellIter = vtk::TakeSmartPointer(input->GetCells()->NewIterator());
  vtkIdType npts;
  const vtkIdType *ptr;
  vtkIdType numOutPts = 0;
  for (cellIter->GoToFirstCell(); !cellIter->IsDoneWithTraversal(); cellIter->GoToNextCell())
    {
    cellIter->GetCurrentCell(npts, ptr);
    for (vtkIdType j = 0 ; j < npts ; j++)
      {
      if (pointMap[ptr[j]] == -1)
        pointMap[ptr[j]] = numOutPts++;
      }
    }

  vtkPoints *newPts = vtkPoints::New(input->GetPoints()->GetDataType());
  newPts->SetNumberOfPoints(numOutPts);
  vtkPointData *inputPD  = input->GetPointData();
  vtkPointData *outputPD = output->GetPointData();
  outputPD->CopyAllocate(inputPD, numOutPts);

  for (vtkIdType j = 0 ; j < numInPts ; j++)
    {
    if (pointMap[j] != -1)
      {
      double pt[3];
      inPts->GetPoint(j, pt);
      newPts->SetPoint(pointMap[j], pt);
      outputPD->CopyData(inputPD, j, pointMap[j]);
      }
    }

  vtkCellData  *inputCD = input->GetCellData();
  vtkCellData  *outputCD = output->GetCellData();
  outputCD->PassData(inputCD);

  vtkIdList *cellIds = vtkIdList::New();

  output->SetPoints(newPts);

  // now work through cells, changing associated point id to coincide
  // with the new ones as specified in the pointmap;

  vtkIdList *oldIds = vtkIdList::New();
  vtkIdList *newIds = vtkIdList::New();
  for (cellIter->GoToFirstCell(); !cellIter->IsDoneWithTraversal(); cellIter->GoToNextCell())
    {
    cellIter->GetCurrentCell(npts, ptr);
    int cellType = input->GetCellType(cellIter->GetCurrentCellId());
    newIds->SetNumberOfIds(npts);
    for (vtkIdType j = 0; j < npts ; j++)
      {
      newIds->SetId(j, pointMap[ptr[j]]);
      }
      output->InsertNextCell(cellType, newIds);
    }

  newPts->Delete();
  oldIds->Delete();
  newIds->Delete();
  cellIds->Delete();
  delete [] pointMap;

  return 1;
}

// ****************************************************************************
//  Method: vtkUnstructuredGridRelevantPointsFilter::FillInputPortInformation
//
// ****************************************************************************
int
vtkUnstructuredGridRelevantPointsFilter::FillInputPortInformation(int,
  vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
  return 1;
}

// ****************************************************************************
//  Method: vtkUnstructuredGridRelevantPointsFilter::PrintSelf
//
// ****************************************************************************
void
vtkUnstructuredGridRelevantPointsFilter::PrintSelf(ostream& os,
  vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
