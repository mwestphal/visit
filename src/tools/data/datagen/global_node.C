// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
// File: global_node.C
//
// Purpose:
//     This program creates a dataset that has global nodes, but not ghost 
//     zones.
//
// Programmer: Hank Childs
// Creation:   October 5, 2004
//
// Modifications:
//
//   Tom Fogal, Sat Feb  7 18:21:23 EST 2009
//   Added missing includes and modernize them.
//
// ****************************************************************************

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <silo.h>
#include <visitstream.h>

// suppress the following since silo uses char * in its API
#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wdeprecated-writable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

void WriteOutMultivars(DBfile *);
void WriteOutDomain(DBfile *, int, int, int);

int NDOMI = 4;
int NNODEI = 64;
int NDOMJ = 4;
int NNODEJ = 64;
int NDOMK = 4;
int NNODEK = 64;

int nmats = 8;

int driver = DB_PDB;

int main(int argc, char *argv[])
{

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "DB_HDF5") == 0)
            driver = DB_HDF5;
        else if (strcmp(argv[i], "DB_PDB") == 0)
            driver = DB_PDB;
        else
           fprintf(stderr,"Uncrecognized driver name \"%s\"\n", argv[i]);
    }

    DBfile *dbfile = DBCreate("global_node.silo", DB_CLOBBER, DB_LOCAL,NULL,driver);

    WriteOutMultivars(dbfile);
    for (int i = 0 ; i < NDOMI ; i++)
        for (int j = 0 ; j < NDOMJ ; j++)
            for (int k = 0 ; k < NDOMK ; k++)
                WriteOutDomain(dbfile, i, j, k);

    DBClose(dbfile);
}

// ****************************************************************************
// Method: WriteOutMultivars
//
// Purpose:
//   Writes out multivar information.  
//
// Programmer: Hank Childs
// Creation:   October 5, 2004
//
// Modifications:
//   Kathleen Bonnell, Wed May 11 17:50:53 PDT 2005
//   Added support for zonal var 'p'.
//
// ****************************************************************************

void
WriteOutMultivars(DBfile *dbfile)
{
    int n_total_domains = NDOMI*NDOMJ*NDOMK;
    char **meshnames  = new char*[n_total_domains];
    char **matnames   = new char*[n_total_domains];
    char **distnames  = new char*[n_total_domains];
    char **pnames     = new char*[n_total_domains];
    int   *meshtypes  = new int[n_total_domains];
    int   *vartypes   = new int[n_total_domains];
   
    for (int i = 0 ; i < n_total_domains ; i++)
    {
        meshnames[i] = new char[128];
        sprintf(meshnames[i], "/block%d/mesh", i);
        matnames[i] = new char[128];
        sprintf(matnames[i], "/block%d/mat", i);
        distnames[i] = new char[128];
        sprintf(distnames[i], "/block%d/dist", i);
        pnames[i] = new char[128];
        sprintf(pnames[i], "/block%d/p", i);
        meshtypes[i] = DB_UCDMESH;
        vartypes[i]  = DB_UCDVAR;
    }
    DBPutMultimesh(dbfile, "mesh", n_total_domains, meshnames, meshtypes,NULL);
    DBPutMultivar(dbfile, "dist", n_total_domains, distnames, vartypes,NULL);
    DBPutMultivar(dbfile, "p", n_total_domains, pnames, vartypes,NULL);
    DBPutMultimat(dbfile, "mat", n_total_domains, matnames, NULL);
}

// ****************************************************************************
// Method: WriteOutDomain
//
// Purpose:
//   Creates and writes out one domain of the mesh. 
//
// Programmer: Hank Childs
// Creation:   October 5, 2004
// 
// Modifications:
//   Kathleen Bonnell, Wed Dec 15 15:03:42 PST 2004
//   Added gzoneno.
//
//   Kathleen Bonnell, Wed May 11 17:50:53 PDT 2005
//   Added support for zonal var 'p'.
//
// ****************************************************************************

void
WriteOutDomain(DBfile *dbfile, int dI, int dJ, int dK)
{
    int  i, j, k;

    int dom = dK*NDOMI*NDOMJ + dJ*NDOMI + dI;
    char dirname[1024];
    sprintf(dirname, "block%d", dom);
    DBMkDir(dbfile, dirname);
    DBSetDir(dbfile, dirname);

    int nx = NNODEI / NDOMI;
    int ny = NNODEJ / NDOMJ;
    int nz = NNODEK / NDOMK;

    float x_start = ((float) dI) / ((float) NDOMI);
    float y_start = ((float) dJ) / ((float) NDOMJ);
    float z_start = ((float) dK) / ((float) NDOMK);
    float rangeX = 1./NDOMI;
    float rangeY = 1./NDOMJ;
    float rangeZ = 1./NDOMK;
    float x_step = rangeX / (nx-1);
    float y_step = rangeY / (ny-1);
    float z_step = rangeZ / (nz-1);

    const char *meshname = "mesh";
    const char *varname  = "dist";
    const char *zvarname  = "p";
    int dims[3];
    dims[0] = nx-1;
    dims[1] = ny-1;
    dims[2] = nz-1;
    int nzones = (nx-1)*(ny-1)*(nz-1);
    int *gzoneno = new int[nzones];
    float *p = new float[nzones];
    int g_istart = dI * (nx-1);
    int g_jstart = dJ * (ny-1);
    int g_kstart = dK * (nz-1);

    //
    // Create zonelist.
    //
    int lzonelist = 8*nzones;
    int *zonelist = new int[lzonelist];
    int zshapetype[1] = { DB_ZONETYPE_HEX };
    int zshapesize[1] = { 8 };
    int zshapecnt[1] = { nzones };
    int idx = 0;
    int gidx = 0;
    for (k = 0 ; k < dims[2] ; k++)
         for (j = 0 ; j < dims[1] ; j++)
            for (i = 0 ; i < dims[0] ; i++)
            {
                int base = k*nx*ny + j*nx + i;


                zonelist[idx++] = base;
                zonelist[idx++] = base+1;
                zonelist[idx++] = base+1+nx;
                zonelist[idx++] = base+nx;
                zonelist[idx++] = base+nx*ny;
                zonelist[idx++] = base+1+nx*ny;
                zonelist[idx++] = base+1+nx+nx*ny;
                zonelist[idx++] = base+nx+nx*ny;

                p[gidx] = i*35.45;

                gzoneno[gidx++] = (k+g_kstart)*(NNODEI-1)*(NNODEJ-1) +
                             (j+g_jstart)*(NNODEI-1) + (i+g_istart);
            }    

    
    DBoptlist *zoptlist = DBMakeOptlist(1);
    DBAddOption(zoptlist, DBOPT_ZONENUM, gzoneno);
    DBPutZonelist2(dbfile, "zl1", nzones, 3, zonelist, lzonelist, 
                  0, 0, 0, zshapetype, zshapesize, zshapecnt, 1, zoptlist);
    DBFreeOptlist(zoptlist);
    delete [] gzoneno;
    delete [] zonelist;

    // 
    // Create the points and the single variable.
    //
    int npts = nx*ny*nz;
    float *x = new float[npts];
    float *y = new float[npts];
    float *z = new float[npts];
    float *dist = new float[npts];
    int *gnodeno = new int[npts];



    for (k = 0 ; k < nz ; k++)
    {
        for (j = 0 ; j < ny ; j++)
        {
            for (i = 0 ; i < nx ; i++)
            {
                int index = k*ny*nx + j*nx + i;
                x[index] = x_start + i*x_step;
                y[index] = y_start + j*y_step;
                z[index] = z_start + k*z_step;
                dist[index] = (x[index]-0.5)*(x[index]-0.5)
                            + (y[index]-0.5)*(y[index]-0.5)
                            + (z[index]-0.5)*(z[index]-0.5);
                dist[index] = sqrt(dist[index]);
                gnodeno[index] = (k+g_kstart)*NNODEI*NNODEJ +
                                 (j+g_jstart)*NNODEI + (i+g_istart);
            }
        }
    }
  
    float *coords[3];
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    DBoptlist *optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_NODENUM, gnodeno);
    DBPutUcdmesh(dbfile, meshname, 3, NULL, coords, npts, nzones, "zl1",
                  NULL, DB_FLOAT, optlist);
    DBPutUcdvar1(dbfile, varname, meshname, dist, npts, NULL, 0,
                  DB_FLOAT, DB_NODECENT, NULL);
    DBPutUcdvar1(dbfile, zvarname, meshname, p, nzones, NULL, 0,
                  DB_FLOAT, DB_ZONECENT, NULL);
    DBFreeOptlist(optlist);
    delete [] gnodeno;

    //
    // Create the materials.
    //
    int *matnos = new int[nmats];
    for (int i = 0 ; i < nmats ; i++)
    {
        matnos[i] = i+1;
    }

    int *matlist = new int[nzones];
    for (int i = 0 ; i < dims[0] ; i++)
        for (int j = 0 ; j < dims[1] ; j++)
            for (int k = 0 ; k < dims[2] ; k++)
            {
                int index = k*ny*nx + j*nx + i;
                float pt[3];
                pt[0] = fabs(0.5-x[index]);
                pt[1] = fabs(0.5-y[index]);
                pt[2] = fabs(0.5-z[index]);
                float prod = pt[0]*pt[1]*pt[2];
                // Max value of prod is 1/8, so multiply by 8.
                float frac = prod * 8. * nmats;
                int mat = (int) frac;
                if (mat >= nmats)
                   mat = nmats-1;
                if (mat <= 0)
                   mat = 1;
                int index2 = k*(ny-1)*(nx-1) + j*(nx-1) + i;
                matlist[index2] = mat;
            }

    DBPutMaterial(dbfile, "mat", meshname, nmats, matnos, matlist, dims, 3,
                  NULL, NULL, NULL, NULL, 0, DB_FLOAT, NULL);
    DBSetDir(dbfile, "..");

    delete [] x;
    delete [] y;
    delete [] z;
    delete [] dist;
    delete [] p;
    delete [] matnos;
    delete [] matlist;
}

