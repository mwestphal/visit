// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "ZooMIR.h"

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>

#include <avtMaterial.h>
#include <avtMixedVariable.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetFromVolume.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTriangulationTables.h>
#include <vtkObjectFactory.h>

#include <VisItArray.h>
#include "BitUtils.h"
#include "RecursiveCellReconstructor.h"
#include "IsovolumeCellReconstructor.h"
#include "ResampledMat.h"

// ****************************************************************************
//  Constructor:  ZooMIR::ZooMIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 20, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 14 15:32:04 EDT 2008
//    Keep track of if the entire mesh was a single material; we'll
//    use this to return the input data set when possible.
//
// ****************************************************************************
ZooMIR::ZooMIR()
{
    singleMat = -1;
    dimension = -1;
    mesh   = NULL;
    outPts = NULL;
    origCoordType = VTK_FLOAT;
}

// ****************************************************************************
//  Destructor:  ZooMIR::~ZooMIR
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 20, 2003
//
// ****************************************************************************
ZooMIR::~ZooMIR()
{
    if (mesh != NULL)
    {
        mesh->Delete();
        mesh = NULL;
    }
    if (outPts != NULL)
    {
        outPts->Delete();
        outPts = NULL;
    }
}

// ****************************************************************************
//  Method:  ZooMIR::Reconstruct3DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 3D.
//
//  Arguments:
//    mesh_orig       the mesh
//    mat_orig        the material
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 18, 2003
//
// ****************************************************************************
bool
ZooMIR::Reconstruct3DMesh(vtkDataSet *mesh_orig, avtMaterial *mat_orig)
{
    return ReconstructMesh(mesh_orig, mat_orig, 3);
}



// ****************************************************************************
//  Method:  ZooMIR::Reconstruct2DMesh
//
//  Purpose:
//    Main method for interface reconstruction in 2D.
//
//  Arguments:
//    mesh_orig       the mesh
//    mat_orig        the material
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 18, 2003
//
// ****************************************************************************
bool
ZooMIR::Reconstruct2DMesh(vtkDataSet *mesh_orig, avtMaterial *mat_orig)
{
    return ReconstructMesh(mesh_orig, mat_orig, 2);
}

// ****************************************************************************
//  Method:  ZooMIR::ReconstructMesh
//
//  Purpose:
//    Main method for interface reconstruction in any dimension.
//
//  Arguments:
//    mesh_orig       the mesh
//    mat_orig        the material
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 20, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Sep 18 10:57:37 PDT 2003
//    Made applicable to any number of dimensions.
//
//    Jeremy Meredith, Tue Oct 14 15:58:57 PDT 2003
//    Added clean-zones-only.
//
//    Jeremy Meredith, Tue Oct 21 11:23:04 PDT 2003
//    Keep track of the number of original materials.  Fixed memory leak.
//
//    Jeremy Meredith, Thu Aug 18 17:59:13 PDT 2005
//    Added a new isovolume algorithm, with adjustable VF cutoff.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Don't send MIRConnectivity to reconstructor classes, it isn't used.
//    Support VTK 9's change in storage for vtkCellArray class:
//    separate connectivity and offsets arrays.
//
// ****************************************************************************
bool
ZooMIR::ReconstructMesh(vtkDataSet *mesh_orig, avtMaterial *mat_orig, int dim)
{
    // check that Reconstruct hasn't already been called
    if (dimension > 0)
    {
        EXCEPTION1(ImproperUseException,
                   "Reconstruct has already been called!");
    }

    // Store the mesh for use later.
    mesh = mesh_orig;
    mesh->Register(NULL);
    nOrigMaterials = mat_orig->GetNMaterials();

    // see if we can perform a clean-zone-only algorithm
    if (options.cleanZonesOnly ||
        mat_orig->GetMixlen() <= 0 ||
        mat_orig->GetMixMat() == NULL)
    {
        noMixedZones = true;
        bool status = ReconstructCleanMesh(mesh, mat_orig);
        return status;
    }

    // start a timer
    int timerHandle = visitTimer->StartTimer();

    // Set the dimensionality
    dimension = dim;

    // Set the connectivity
    MIRConnectivity conn;
    conn.SetUpConnectivity(mesh);

    // extract coordinate arrays
    SetUpCoords();

    // Pack the material
    avtMaterial *mat = mat_orig->CreatePackedMaterial();

    // For iteration, create another copy of the material to use
    // for the desired VF's.  CreatePackedMaterial efficiently
    // creates a copy of an already-packed material.
    avtMaterial *matCopy = options.numIterations > 0 ?
                              mat->CreatePackedMaterial() : NULL;

    mapMatToUsedMat = mat_orig->GetMapMatToUsedMat();
    mapUsedMatToMat = mat_orig->GetMapUsedMatToMat();

    // Get some attributes
    noMixedZones= false;
    nMaterials  = mat->GetNMaterials();

    int nCells  = mesh->GetNumberOfCells();
    int nPoints = mesh->GetNumberOfPoints();

    // resample the material volume fractions to the nodes
    for (int iter = 0; iter < options.numIterations+1; iter++)
    {
        coordsList.clear();
        zonesList.clear();
        indexList.clear();

        ResampledMat rm(nCells, nPoints, mat, &conn);
        rm.Resample();

        // loop over the cells and do the reconstruction
        int timerHandle2 = visitTimer->StartTimer();

        coordsList.reserve(nPoints/4);
        zonesList.reserve(int(float(nCells)*1.5));
        indexList.reserve(int(float(nCells)*10));

        CellReconstructor *cr;
        if (options.algorithm == 1) // Recursive Clipping
        {
            cr = new RecursiveCellReconstructor(mesh, mat, rm, nPoints,
                                                nCells, *this);
        }
        else // algorithm == 2 // Isovolume
        {
            cr = new IsovolumeCellReconstructor(mesh, mat, rm, nPoints,
                                                nCells, *this);
        }

        // actualVols contains reconstructed volume/areas per material
        double *actualVolStorage = new double[nMaterials];
        double *actualVols = (options.numIterations>0) ? actualVolStorage : NULL;
        // tmpVols is like actualVols, but for tessellated sub-cells
        double *tmpVolStorage = new double[nMaterials];
        double *tmpVols = (options.numIterations>0) ? tmpVolStorage : NULL;

        double maxdiff = 0;
        for (int c = 0 ; c < nCells ; c++)
        {
            int  celltype = conn.celltype[c];
            vtkIdType nids = conn.offsets[c+1] - conn.offsets[c];
            vtkIdType *ids = &conn.connectivity[conn.offsets[c]];

            double totalvolume = 0;
            if (nids > MAX_NODES_PER_ZONE)
            {
                // No zoo shape has more than 8 nodes/zone except for
                // polygons. We have tables up to octagons. To support
                // 9-sided and larger polygons, we just tessellate
                // into triangles. Note that for iteration, we need
                // to carefully accumulate reconstructed areas.
                if (actualVols)
                    for (int m=0; m<nMaterials; ++m)
                        actualVols[m] = 0;

                vtkIdType tmpids[3];
                for (int i=1; i<nids-1; i++)
                {
                    tmpids[0] = ids[0];
                    tmpids[1] = ids[i];
                    tmpids[2] = ids[i+1];
                    // accumulate total volume from sub-cells
                    totalvolume += cr->ReconstructCell(c, VTK_TRIANGLE, 3,
                                                       tmpids, tmpVols);
                    // accumulate sub-cell per-mat vol/areas into whole-cell
                    if (actualVols)
                        for (int m=0; m<nMaterials; ++m)
                            actualVols[m] += tmpVols[m];
                }
            }
            else
            {
                totalvolume = cr->ReconstructCell(c, celltype, nids, ids,
                                                  actualVols);
            }

            if (options.numIterations > 0 &&
                iter < options.numIterations)
            {
                std::vector<float> neededVFs, triedVFs;
                matCopy->GetVolFracsForZone(c,neededVFs);
                mat->GetVolFracsForZone(c,triedVFs);
                for (int m=0; m<nMaterials; m++)
                {
                    float wanted = neededVFs[m];
                    float tried  = triedVFs[m];
                    float got    = actualVols[m] / totalvolume;
                    float diff   = wanted - got;

                    if (diff != 0)
                    {
                        float newval = tried + options.iterationDamping * diff;
                        mat->SetVolFracForZoneAndMat(c,m,newval);
                        if (diff > maxdiff)
                            maxdiff = diff;
                    }
                }
            }
        }

        visitTimer->StopTimer(timerHandle2, "MIR: Cell clipping");
        delete [] actualVolStorage;
        delete [] tmpVolStorage;
        delete cr;
    }

    visitTimer->StopTimer(timerHandle, "Full MIR reconstruction");
    visitTimer->DumpTimings();

    if (matCopy)
        delete matCopy;
    delete mat;
    return true;
}

// ****************************************************************************
//  Method:  ZooMIR::GetDataset
//
//  Purpose:
//    Return the cells that correspond to the given materials.  Do the right
//    thing with the various data arrays, etc.
//
//  Arguments:
//    mats        the materials to select
//    ds          the original dataset
//    mixvars     the mixed variables
//    doMats      flag to write out the material numbers as a new zonal array
//
//  Note: taken from TetMIR's implementation, but assumes the original
//  nodes were NOT added to coordsList.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 20, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Oct 15 16:47:49 PDT 2003
//    Allowed a material to get passed in.  This is completely optional and
//    used only for cleanZonesOnly.  Also updated cleanZonesOnly to not only
//    return the mixed zones as clean zones (with a material index one beyond
//    the number of materials), but for the cleanZonesOnly support to be
//    smart about which mixed zones to return.
//
//    Jeremy Meredith, Tue Oct 21 11:22:30 PDT 2003
//    Use number of original materials for the clean-zone-material instead
//    of the number of used materials.
//
//    Hank Childs, Wed Jan 14 08:12:02 PST 2004
//    Construct the output unstructured grid more efficiently.
//
//    Hank Childs, Fri Jan 30 08:46:07 PST 2004
//    Removed unnecessary Allocate call.
//
//    Jeremy Meredith, Wed Jun 16 11:05:28 PDT 2004
//    Original node numbers array was being interpolated.  We now just slap
//    a -1 in for new nodes instead of trying to interpolate them.
//
//    Jeremy Meredith, Tue Oct 14 15:32:04 EDT 2008
//    If the entire mesh was truly a single material, there are two new fast
//    paths:  just return the input mesh (if that material is selected)
//    or return nothing (if the material wasn't seelcted).  This allows
//    the output to be a rectilinear, or other non-unstructured, data set.
//
//    Mark C. Miller, Tue Jan 15 13:22:29 PST 2013
//    Adjusted logic to overwrite mix values to use SetTuple instead of
//    assuming a float* array.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK 9's change in storage for vtkCellArray class:
//    separate connectivity and offsets arrays.
//
// ****************************************************************************
vtkDataSet *
ZooMIR::GetDataset(std::vector<int> mats, vtkDataSet *ds,
                   std::vector<avtMixedVariable*> mixvars, bool doMats,
                   avtMaterial *mat)
{
    int timerHandle = visitTimer->StartTimer();
    bool doAllMats = mats.empty();

    //
    // Start off by determining which materials we should reconstruct and
    // which we should leave out.
    //
    bool *matFlag = new bool[nMaterials+1];
    if (!doAllMats)
    {
        for (int i = 0; i < nMaterials+1; i++)
            matFlag[i] = false;
        for (size_t i = 0; i < mats.size(); i++)
        {
            int origmatno = mats[i];
            if (origmatno == nOrigMaterials)
            {
                matFlag[nMaterials] = true;
            }
            else
            {
                int usedmatno = mapMatToUsedMat[origmatno];
                if (usedmatno != -1)
                    matFlag[usedmatno] = true;
            }
        }
    }
    else
    {
        for (int i = 0; i < nMaterials+1; i++)
            matFlag[i] = true;
    }

    //
    // If this domain was truly clean, we can re-use the input mesh
    //
    if (singleMat >= 0)
    {
        if (matFlag[singleMat])
        {
            // Since if we make a copy, we get corrupt ->Links
            // for an unstructured grid, we're just going to add
            // avtSubsets to the input mesh and
            vtkDataSet *outmesh = ds;

            // (this was the other option: ----->>>
            //vtkDataSet *outmesh = ds->NewInstance();
            //outmesh->ShallowCopy(ds);
            //if (outmesh->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
            //    ((vtkUnstructuredGrid*)outmesh)->BuildLinks();
            // <<----- )

            if (doMats)
            {
                //
                // Add the avtSubsets array
                //
                int ncells = outmesh->GetNumberOfCells();
                vtkIntArray *outmat = vtkIntArray::New();
                outmat->SetName("avtSubsets");
                outmat->SetNumberOfTuples(ncells);
                int *buff = outmat->GetPointer(0);
                for (int i=0; i<ncells; i++)
                    buff[i] = singleMat;
                outmesh->GetCellData()->AddArray(outmat);
                outmat->Delete();
            }
            outmesh->Register(NULL);
            visitTimer->StopTimer(timerHandle,
                                  "MIR: Getting *completely* clean dataset");
            visitTimer->DumpTimings();
            delete [] matFlag;
            return outmesh;
        }
        else
        {
            visitTimer->StopTimer(timerHandle,
                                  "MIR: Getting *empty* clean dataset");
            visitTimer->DumpTimings();
            delete [] matFlag;
            return NULL;
        }
    }

    //
    // Now count up the total number of cells we will have.
    //
    int ntotalcells = zonesList.size();
    int ncells = 0;
    int *cellList = new int[ntotalcells];
    int totalsize = 0;
    for (int c = 0; c < ntotalcells; c++)
    {
        int matno = zonesList[c].mat;
        if (matno >= 0)
        {
            if (matFlag[matno])
            {
                cellList[ncells++] = c;
                totalsize += zonesList[c].nnodes;
            }
        }
        else if (!mat)
        {
            if (matFlag[nMaterials])
            {
                cellList[ncells++] = c;
                totalsize += zonesList[c].nnodes;
            }
        }
        else
        {
            bool match = false;
            int  mixIndex = -matno - 1;
            while (mixIndex >= 0 && !match)
            {
                matno = mat->GetMixMat()[mixIndex];

                if (matFlag[matno])
                    match = true;

                mixIndex = mat->GetMixNext()[mixIndex] - 1;
            }
            if (match)
            {
                cellList[ncells++] = c;
                totalsize += zonesList[c].nnodes;
            }
        }
    }

    //
    // Instantiate the output dataset (-> VTK magic).
    //
    vtkUnstructuredGrid   *rv  = vtkUnstructuredGrid::New();
    rv->GetFieldData()->PassData(ds->GetFieldData());

    //
    // Set up the coordinate array.
    //
    int newNPoints = coordsList.size();
    int npoints = origNPoints + newNPoints;
    if (outPts == NULL)
    {
        outPts = vtkPoints::New(origCoordType);
        outPts->SetNumberOfPoints(npoints);
        if(origCoordType == VTK_FLOAT)
        {
            float *pts_buff = (float *) outPts->GetVoidPointer(0);
            for (int i=0; i<origNPoints; i++)
            {
                *pts_buff++ = static_cast<float>(origXCoords[i]);
                *pts_buff++ = static_cast<float>(origYCoords[i]);
                *pts_buff++ = static_cast<float>(origZCoords[i]);
            }
            for (int i=0; i<newNPoints; i++)
            {
                *pts_buff++ = static_cast<float>(coordsList[i].x);
                *pts_buff++ = static_cast<float>(coordsList[i].y);
                *pts_buff++ = static_cast<float>(coordsList[i].z);
            }
        }
        else if(origCoordType == VTK_DOUBLE)
        {
            double *pts_buff = (double *) outPts->GetVoidPointer(0);
            for (int i=0; i<origNPoints; i++)
            {
                *pts_buff++ = origXCoords[i];
                *pts_buff++ = origYCoords[i];
                *pts_buff++ = origZCoords[i];
            }
            for (int i=0; i<newNPoints; i++)
            {
                *pts_buff++ = coordsList[i].x;
                *pts_buff++ = coordsList[i].y;
                *pts_buff++ = coordsList[i].z;
            }
        }
        else
        {
            outPts->Delete();
            rv->Delete();
            EXCEPTION0(ImproperUseException);
        }
    }
    rv->SetPoints(outPts);

    //
    // Now insert the connectivity and offsets array.
    //

    // holds the connectivity
    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(totalsize);
    vtkIdType *nl = nlist->GetPointer(0);

    // holds the offsets
    vtkIdTypeArray *olist = vtkIdTypeArray::New();
    olist->SetNumberOfValues(ncells+1);
    vtkIdType *ol = olist->GetPointer(0);
    // first entry is always 0
    *ol++ = 0;
    // Subsequent offsets are incremented by the number of ids in
    // the current cell, so set up a holder for that increment.
    vtkIdType offset = 0;
    // The last entry in offsets will hold the size of connectivity

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(ncells);
    unsigned char *ct = cellTypes->GetPointer(0);

    for (int i=0; i<ncells; i++)
    {
        int c = cellList[i];

        *ct++ = zonesList[c].celltype;

        const int nnodes = zonesList[c].nnodes;
        const vtkIdType *indices = &indexList[zonesList[c].startindex];
        for (int j=0; j<nnodes; j++)
            *nl++ = indices[j];
        offset += nnodes;
        *ol++ = offset;
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetData(olist, nlist);
    nlist->Delete();
    olist->Delete();

    rv->SetCells(cellTypes, cells);
    cellTypes->Delete();
    cells->Delete();

    //
    // Copy over all node-centered data.
    //
    vtkPointData *outpd = rv->GetPointData();
    vtkPointData *inpd  = ds->GetPointData();
    if (inpd->GetNumberOfArrays() > 0)
    {
        outpd->CopyAllocate(inpd, npoints);
        int outIndex = 0;
        //
        // For each point, copy over the original point data if the
        // reconstructed coordinate corresponds to a point in the dataset.
        // If the point is new, interpolate the values from the zone it
        // comes from.
        //
        for (int i=0; i<origNPoints; i++)
        {
            outpd->CopyData(inpd, i, outIndex++);
        }

        int newPtStartingIndex = outIndex;
        for (int i=0; i<newNPoints; i++)
        {
            ReconstructedCoord &coord = coordsList[i];
            vtkCell *cell = mesh->GetCell(coord.origzone);
            outpd->InterpolatePoint(inpd, outIndex++, cell->GetPointIds(),
                                    coord.weight);
        }

        //
        // The original node numbers should *not* be interpolated!
        // Throw a -1 in there for new nodes as a flag.
        //
        vtkDataArray *origNodes = outpd->GetArray("avtOriginalNodeNumbers");
        if (origNodes)
        {
            int  multiplier = origNodes->GetNumberOfComponents();
            int  offset     = (multiplier == 2) ? 1 : 0;
            int *origNodeArray = (int*)origNodes->GetVoidPointer(0);
            for (int i=0; i<newNPoints; i++)
            {
                origNodeArray[(newPtStartingIndex+i)*multiplier + offset] = -1;
            }
        }
    }
    //
    // Copy over all the cell-centered data.  The logic gets awfully confusing
    // when using the VTK convenience methods *and* we have mixed variables,
    // so just do as normal and we will copy over the mixed values later.
    //
    vtkCellData *outcd = rv->GetCellData();
    vtkCellData *incd  = ds->GetCellData();
    if (incd->GetNumberOfArrays() > 0)
    {
        outcd->CopyAllocate(incd, ncells);
        for (int i=0; i<ncells; i++)
        {
            int c = cellList[i];
            int origzone = zonesList[c].origzone;
            outcd->CopyData(incd, origzone, i);
        }
    }

    //
    // Now go and write over the mixed part of the mixed variables.  The non-
    // mixed part was already copied over in the last operation.
    //
    for (size_t i=0; i<mixvars.size(); i++)
    {
        avtMixedVariable *mv = mixvars[i];
        if (mv == NULL)
        {
            continue;
        }
        vtkDataArray *arr = outcd->GetArray(mv->GetVarname().c_str());
        if (arr == NULL)
        {
            debug1 << "INTERNAL ERROR IN MIR.  Asked to reconstruct a variable"
                   << " with mixed elements,\nbut could not find the original "
                   << "variable array." << endl;
            debug1 << "The mixed variable is " << mv->GetVarname().c_str() << endl;
            debug1 << "Variables in the VTK dataset are: ";
            for (int j = 0 ; j < outcd->GetNumberOfArrays() ; j++)
            {
                debug1 << outcd->GetArray(j)->GetName() << ", ";
            }
            debug1 << endl;
            continue;
        }
        if (arr->GetNumberOfComponents() != 1)
        {
            debug1 << "Can not operate on mixed vars that aren't scalars."
                   << endl;
            continue;
        }
        const float *buffer = mv->GetBuffer();
        debug4 << "Overwriting mixed values for " << arr->GetName() << endl;
        int nvals = 0;
        for (int j=0; j<ncells; j++)
        {
            int mix_index = zonesList[cellList[j]].mix_index;
            if (mix_index >= 0)
            {
                arr->SetTuple(j, &buffer[mix_index]);
                nvals++;
            }
        }
        debug4 << "Overwrote " << nvals << " values (by tet, not necessarily "
               << "by original zone)" << endl;
    }

    if (doMats)
    {
        //
        // Add an array that contains the material for each zone (which is now
        // clean after reconstruction).
        //
        vtkIntArray *outmat = vtkIntArray::New();
        outmat->SetName("avtSubsets");
        outmat->SetNumberOfTuples(ncells);
        int *buff = outmat->GetPointer(0);
        for (int i=0; i<ncells; i++)
        {
            int matno = zonesList[cellList[i]].mat;
            buff[i] = matno < 0 ? nOrigMaterials : mapUsedMatToMat[matno];
        }
        rv->GetCellData()->AddArray(outmat);
        outmat->Delete();
    }

    delete [] matFlag;
    delete [] cellList;

    visitTimer->StopTimer(timerHandle, "MIR: Getting clean dataset");
    visitTimer->DumpTimings();

    return rv;
}

// ****************************************************************************
//  Method:  ZooMIR::ReconstructCleanMesh
//
//  Purpose:
//    Main loop for interface reconstruction for any clean mesh.
//
//  Arguments:
//    mesh       the mesh
//    mat        the material
//    conn       the connectivity
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 20, 2003
//
//  Note:  Copied from TetMIR's implementation
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 14 15:38:55 EDT 2008
//    If there are no mixed materials (i.e. we're not just pretending
//    to reconstruct the mesh as if it were clean), and if there is
//    only a single material, then trigger the single-material short
//    circuit.  (big speedup, less memory)
//
//    Mark C. Miller, Tue Oct 21 23:11:24 PDT 2008
//    Fixed bug where singleMat would get assigned matlist[0] even if matlist
//    was never allocated (ncells == 0).
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK 9's change in storage for vtkCellArray Class:
//    separate connectivity and offsets arrays.
//
// ****************************************************************************
bool
ZooMIR::ReconstructCleanMesh(vtkDataSet *mesh, avtMaterial *mat)
{
    int timerHandle = visitTimer->StartTimer();

    // no need to pack, so fake that part
    nMaterials = mat->GetNMaterials();
    mapMatToUsedMat.resize(mat->GetNMaterials(), -1);
    mapUsedMatToMat.resize(mat->GetNMaterials(), -1);
    for (int m=0; m<mat->GetNMaterials(); m++)
    {
        mapMatToUsedMat[m] = m;
        mapUsedMatToMat[m] = m;
    }

    // Can we skip the reconstruction entirely?
    if (mat->GetMixlen() <= 0 ||
        mat->GetMixMat() == NULL)
    {
        int ncells = mesh->GetNumberOfCells();
        const int *matlist = mat->GetMatlist();
        singleMat = ncells > 0 ? matlist[0] : -1;
        for (int c=1; c<ncells; c++)
        {
            if (matlist[c] != singleMat)
            {
                singleMat = -1;
                break;
            }
        }
        if (singleMat >= 0)
        {
            visitTimer->StopTimer(timerHandle,
                               "MIR: Reconstructing clean mesh: one mat only");
            visitTimer->DumpTimings();
            return true;
        }
    }

    // Set the connectivity
    MIRConnectivity conn;
    conn.SetUpConnectivity(mesh);

    // extract coords
    SetUpCoords();

    // extract cells
    int        nCells  = conn.ncells;
    const int *matlist = mat->GetMatlist();
    zonesList.resize(nCells);
    for (int c=0; c<nCells; c++)
    {
        // using c+1 for offsets is safe because offsets size is nCells+1
        vtkIdType nIds = conn.offsets[c+1]-conn.offsets[c];

        ReconstructedZone &zone = zonesList[c];
        zone.origzone   = c;
        zone.mat        = matlist[c];
        zone.celltype   = conn.celltype[c];
        zone.nnodes     = nIds;
        zone.startindex = indexList.size();
        zone.mix_index  = -1;

        const vtkIdType *ids  = &conn.connectivity[conn.offsets[c]];
        for (int n=0; n<nIds; n++)
            indexList.push_back(ids[n]);
    }

    visitTimer->StopTimer(timerHandle, "MIR: Reconstructing clean mesh");
    visitTimer->DumpTimings();

    return true;
}


// ****************************************************************************
//  Function: SetUpCoords
//
//  Purpose:
//      Sets up the coordinates array.  Avoid using any VTK calls in its inner
//      loop.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Apr 11 21:01:06 PDT 2012
//    double coordinates.
//
// ****************************************************************************

void
ZooMIR::SetUpCoords()
{
    int timerHandle1 = visitTimer->StartTimer();

    int nPoints = mesh->GetNumberOfPoints();
    int i, j, k;

    origNPoints = nPoints;
    origXCoords.resize(nPoints);
    origYCoords.resize(nPoints);
    origZCoords.resize(nPoints);

    int dstype = mesh->GetDataObjectType();
    if (dstype == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) mesh;
        vtkDataArray *xc = rgrid->GetXCoordinates();
        int nx = xc->GetNumberOfTuples();
        double *x = new double[nx];
        for (i = 0 ; i < nx ; i++)
        {
            x[i] = xc->GetTuple1(i);
        }
        vtkDataArray *yc = rgrid->GetYCoordinates();
        int ny = yc->GetNumberOfTuples();
        double *y = new double[ny];
        for (i = 0 ; i < ny ; i++)
        {
            y[i] = yc->GetTuple1(i);
        }
        vtkDataArray *zc = rgrid->GetZCoordinates();
        int nz = zc->GetNumberOfTuples();
        double *z = new double[nz];
        for (i = 0 ; i < nz ; i++)
        {
            z[i] = zc->GetTuple1(i);
        }

        int pt = 0;
        for (k = 0 ; k < nz ; k++)
        {
            for (j = 0 ; j < ny ; j++)
            {
                for (i = 0 ; i < nx ; i++)
                {
                    origXCoords[pt] = x[i];
                    origYCoords[pt] = y[j];
                    origZCoords[pt] = z[k];
                    pt++;
                }
            }
        }
        delete [] x;
        delete [] y;
        delete [] z;
        origCoordType = rgrid->GetXCoordinates()->GetDataType();
    }
    else
    {
        vtkPointSet *ps = (vtkPointSet *) mesh;
        origCoordType = ps->GetPoints()->GetDataType();
        if(origCoordType == VTK_FLOAT)
        {
            const float *ptr = (float *) ps->GetPoints()->GetVoidPointer(0);
            for (int n=0; n<nPoints; n++)
            {
                origXCoords[n] = static_cast<double>(*ptr++);
                origYCoords[n] = static_cast<double>(*ptr++);
                origZCoords[n] = static_cast<double>(*ptr++);
            }
        }
        else if(origCoordType == VTK_DOUBLE)
        {
            const double *ptr = (double *) ps->GetPoints()->GetVoidPointer(0);
            for (int n=0; n<nPoints; n++)
            {
                origXCoords[n] = *ptr++;
                origYCoords[n] = *ptr++;
                origZCoords[n] = *ptr++;
            }
        }
        else
        {
            EXCEPTION0(ImproperUseException);
        }
    }

    visitTimer->StopTimer(timerHandle1, "MIR: Copying coordinate list");
    visitTimer->DumpTimings();
}



// ****************************************************************************
// Methods:     EdgeHash routines from vtkDataSetFromVolume
// Programmer:  Hank Childs
// Date:        June 9, 2003
//
// Note: Jeremy Meredith, September 15, 2003
//       Taken blatantly from vtkDataSetFromVolume.  These should be
//       refactored into a common place.
//
// Modifications:
//    Jeremy Meredith, Thu Aug 18 17:59:46 PDT 2005
//    Added a new member for each edge -- the material number.  This is
//    because we are now also using this for an isovolume MIR method where
//    we do NOT want the edge points to be reused across materials.
//
// ****************************************************************************

ZooMIR::EdgeHashEntryMemoryManager::EdgeHashEntryMemoryManager()
{
    freeEntryindex = 0;
}


ZooMIR::EdgeHashEntryMemoryManager::~EdgeHashEntryMemoryManager()
{
    int npools = edgeHashEntrypool.size();
    for (int i = 0 ; i < npools ; i++)
    {
        EdgeHashEntry *pool = edgeHashEntrypool[i];
        delete [] pool;
    }
}


void
ZooMIR::EdgeHashEntryMemoryManager::AllocateEdgeHashEntryPool(void)
{
    if (freeEntryindex == 0)
    {
        EdgeHashEntry *newlist = new EdgeHashEntry[POOL_SIZE];
        edgeHashEntrypool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            freeEntrylist[i] = &(newlist[i]);
        }
        freeEntryindex = POOL_SIZE;
    }
}


ZooMIR::EdgeHashTable::EdgeHashTable(int nh)
{
    nHashes = nh;
    hashes = new EdgeHashEntry*[nHashes];
    for (int i = 0 ; i < nHashes ; i++)
        hashes[i] = NULL;
}


ZooMIR::EdgeHashTable::~EdgeHashTable()
{
    delete [] hashes;
}


int
ZooMIR::EdgeHashTable::GetKey(int p1, int p2, int m)
{
    int rv = ((m*521 + p1*18457 + p2*234749) % nHashes);

    // In case of overflows and modulo with negative numbers.
    if (rv < 0)
       rv += nHashes;

    return rv;
}


ZooMIR::EdgeHashEntry *
ZooMIR::EdgeHashTable::GetEdge(int ap1, int ap2, int m)
{
    int p1, p2;
    if (ap2 < ap1)
    {
        p1 = ap2;
        p2 = ap1;
    }
    else
    {
        p1 = ap1;
        p2 = ap2;
    }

    int key = GetKey(p1, p2, m);

    //
    // See if we have any matches in the current hashes.
    //
    EdgeHashEntry *cur = hashes[key];
    while (cur != NULL)
    {
        if (cur->IsMatch(p1, p2, m))
        {
            //
            // We found a match.
            //
            return cur;
        }
        cur = cur->GetNext();
    }

    //
    // There was no match.  We will have to add a new entry.
    //
    EdgeHashEntry *new_one = emm.GetFreeEdgeHashEntry();

    new_one->SetNext(hashes[key]);
    new_one->SetEndpoints(p1, p2);
    new_one->SetMatNo(m);
    hashes[key] = new_one;

    return new_one;
}
