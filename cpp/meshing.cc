//
// Created by klaus on 08.03.19.
//

#include "meshing.h"
#include "logger.h"
#include "eigen.h"
#define VOID void
#define REAL double
#include "triangle/triangle.h"

using namespace std;
using namespace Eigen;

Meshing::Meshing()
{
}

template<>
void Meshing::generate<2, 2>(Mesh<2, 2>& mesh)
{
   triangulateio triio;
   triio.pointlist = &mesh.getCoords()[0];
   const size_t npts = mesh.getCoords().size() / 2;
   triio.numberofpoints = npts;
   triio.numberofsegments = 0;
   vector<int> edgemarks(npts, 0);
   triio.edgemarkerlist = &edgemarks[0];
   vector<int> pointmarks(npts, 0);
   triio.pointmarkerlist = &pointmarks[0];
   triio.numberofpointattributes = 0;
   triio.pointmarkerlist = nullptr;
   triio.numberofholes = 0;
   triio.numberofregions = 0;

   triangulateio triout;
   triout.pointlist = nullptr;            /* Not needed if -N switch used. */
   /* Not needed if -N switch used or number of point attributes is zero: */
   triout.pointattributelist = nullptr;
   triout.pointmarkerlist = nullptr; /* Not needed if -N or -B switch used. */
   triout.trianglelist = nullptr;          /* Not needed if -E switch used. */
   /* Not needed if -E switch used or number of triangle attributes is zero: */
   triout.triangleattributelist = nullptr;
   triout.neighborlist = nullptr;         /* Needed only if -n switch used. */
   /* Needed only if segments are output (-p or -c) and -P not used: */
   triout.segmentlist = nullptr;
   /* Needed only if segments are output (-p or -c) and -P and -B not used: */
   triout.segmentmarkerlist = nullptr;
   triout.edgelist = nullptr;             /* Needed only if -e switch used. */
   triout.edgemarkerlist = nullptr;   /* Needed if -e used and -B not used. */

   triangulateio vout;
   vout.pointlist = nullptr;            /* Not needed if -N switch used. */
   /* Not needed if -N switch used or number of point attributes is zero: */
   vout.pointattributelist = nullptr;
   vout.pointmarkerlist = nullptr; /* Not needed if -N or -B switch used. */
   vout.trianglelist = nullptr;          /* Not needed if -E switch used. */
   /* Not needed if -E switch used or number of triangle attributes is zero: */
   vout.triangleattributelist = nullptr;
   vout.neighborlist = nullptr;         /* Needed only if -n switch used. */
   /* Needed only if segments are output (-p or -c) and -P not used: */
   vout.segmentlist = nullptr;
   /* Needed only if segments are output (-p or -c) and -P and -B not used: */
   vout.segmentmarkerlist = nullptr;
   vout.edgelist = nullptr;             /* Needed only if -e switch used. */
   vout.edgemarkerlist = nullptr;   /* Needed if -e used and -B not used. */

   triangulate((char *) "vpeczqQ", &triio, &triout, &vout);

   copy(triout.edgelist, triout.edgelist + triout.numberofedges,
           back_inserter(*mesh.edges_lst));
   copy(triout.trianglelist, triout.trianglelist + triout.numberoftriangles,
           back_inserter(*mesh.faces_lst));

   Mesh<2, 1>* hull = dynamic_cast<Mesh<2, 1>*>(mesh.getHull());
   for (size_t iv = 0; iv < triout.numberofpoints; iv++) {
      mesh.getOrCreateVertexByID(iv);
      if (triout.pointmarkerlist[iv] > 0)
         hull->getOrCreateVertexByID(iv);
   }
   LineHash linhash;
   for (ID eid = 0; eid < triout.numberofedges; ++eid) {
      mesh.insertEdge(eid);
      if (triout.edgemarkerlist[eid] > 0)
         hull->insertEdge(eid);
   }
   for (ID fid = 0; fid < triout.numberoftriangles; ++fid) {
      mesh.insertFace(fid);
   }
   /*triangulate((char *) "v", &triio, &triout, &vout);
   cout << "Voronoi done" << endl;
   cout.flush();*/
}

template<>
void Meshing::generate<3, 3>(Mesh<3, 3>& mesh)
{

}
