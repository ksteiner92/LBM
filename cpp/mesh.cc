//
// Created by klaus on 06.01.19.
//

#include "mesh.h"

using namespace std;
using namespace Eigen;

template<uint Dim>
Mesh<Dim, 0>::Mesh(vector<Matrix<double, Dim, 1>>* points) : points(points)
{
   vertices_owner = make_unique<vector<unique_ptr<Vertex<Dim>>>>();
   vertices = vertices_owner.get();
   const size_t npts = points->size();
   vertices->reserve(npts);
}

template<uint Dim>
const Matrix<double, Dim, 1>& Mesh<Dim, 0>::getPoint(size_t idx) const
{
   if (idx >= points->size())
      throw out_of_range("Index out of range");
   return (*points)[idx];
}

template<uint Dim>
vector<Matrix<double, Dim, 1>>* Mesh<Dim, 0>::getPoints() const
{
   return points;
}

template<uint Dim>
size_t Mesh<Dim, 0>::getNumPoints() const
{
   return points->size();
}

template<uint Dim>
Vertex<Dim>* Mesh<Dim, 0>::getVertex(size_t idx) const
{
   if (idx < refvertices.size())
      return (*vertices)[refvertices[idx]].get();
   return nullptr;
}

template<uint Dim>
Vertex<Dim>* Mesh<Dim, 0>::getVertexByID(size_t id) const
{
   if (!binary_search(refvertices.begin(), refvertices.end(), id))
      return nullptr;
   if (id < vertices->size())
      return (*vertices)[id].get();
   return nullptr;
}

template<uint Dim>
Vertex<Dim>* Mesh<Dim, 0>::getOrCreateVertexByID(size_t id)
{
   Vertex<Dim>* vertex = getVertexByID(id);
   if (vertex == nullptr) {
      if (id >= points->size())
         throw out_of_range("Vertex index out of range");
      vertices->push_back(make_unique<Vertex<Dim>>(Vertex<Dim>(id, id)));
      vertex = vertices->back().get();
   }
   if (!binary_search(refvertices.begin(), refvertices.end(), id))
      insertSorted(refvertices, id);
   return vertex;
}

template<uint Dim>
size_t Mesh<Dim, 0>::getNumVertices() const
{
   return refvertices.size();
}

template<uint Dim>
Mesh<Dim, 1>::Mesh(vector<Matrix<double, Dim, 1>>* points) : Mesh<Dim, 0>(points)
{
   edges_owner = make_unique<vector<unique_ptr<Edge<Dim>>>>();
   edges = edges_owner.get();
   vhash2id_owner = make_unique<unordered_map<ullong, size_t>>();
   vhash2id = vhash2id_owner.get();
   hull = make_unique<Mesh<Dim, 0>>(this);
}

template<uint Dim>
Edge<Dim>* Mesh<Dim, 1>::getEdge(size_t idx) const
{
   if (idx < refedges.size())
      return (*edges)[refedges[idx]].get();
   return nullptr;
}

template<uint Dim>
Edge<Dim>* Mesh<Dim, 1>::getEdgeByID(size_t id) const
{
   if (!binary_search(refedges.begin(), refedges.end(), id))
      return nullptr;
   if (id < edges->size())
      return (*edges)[id].get();
   return nullptr;
}

template<uint Dim>
Edge<Dim>* Mesh<Dim, 1>::getEdge(size_t vid1, size_t vid2) const
{
   auto it = vhash2id->find(linhash(make_tuple(vid1, vid2)));
   if (it != vhash2id->end())
      return getEdgeByID(it->second);
   else {
      it = vhash2id->find(linhash(make_tuple(vid2, vid1)));
      if (it != vhash2id->end())
         return getEdgeByID(it->second);
   }
   return nullptr;
}

template<uint Dim>
Edge<Dim> *Mesh<Dim, 1>::getOrCreateEdgeByID(size_t id)
{
   Edge<Dim>* edge = nullptr;
   if (id < edges->size()) {
      edge = (*edges)[id].get();
      if (!binary_search(refedges.begin(), refedges.end(), id))
         Mesh<Dim, 0>::insertSorted(refedges, edge->getID());
      Mesh<Dim, 0>::getOrCreateVertexByID((*edge)[0]);
      Mesh<Dim, 0>::getOrCreateVertexByID((*edge)[1]);
   }
   return edge;
}

template<uint Dim>
Edge<Dim> *Mesh<Dim, 1>::getOrCreateEdge(size_t vid1, size_t vid2)
{
   Edge<Dim>* edge = nullptr;
   const ullong key = linhash(make_tuple(vid1, vid2));
   auto it = vhash2id->find(key);
   if (it == vhash2id->end()) {
      it = vhash2id->find(linhash(make_tuple(vid2, vid1)));
      if (it == vhash2id->end()) {
         Vertex<Dim>* v1 = Mesh<Dim, 0>::getOrCreateVertexByID(vid1);
         if (v1 == nullptr)
            throw out_of_range("Vertex index 1 out of range");
         Vertex<Dim>* v2 = Mesh<Dim, 0>::getOrCreateVertexByID(vid2);
         if (v2 == nullptr)
            throw out_of_range("Vertex index 2 out of range");
         const size_t id = edges->size();
         edges->push_back(make_unique<Edge<Dim>>(Edge<Dim>({v1, v2}, id)));
         edge = edges->back().get();
         (*vhash2id)[key] = id;
         vertex2edge.insert({vid1, edge});
         vertex2edge.insert({vid2, edge});
      } else
         edge = (*edges)[it->second].get();
   } else
      edge = (*edges)[it->second].get();
   const size_t id = edge->getID();
   if (!binary_search(refedges.begin(), refedges.end(), id))
      Mesh<Dim, 0>::insertSorted(refedges, id);
   return edge;
}

template<uint Dim>
size_t Mesh<Dim, 1>::getNumEdges() const
{
   return refedges.size();
}

template<uint Dim>
Mesh<Dim, 0>* Mesh<Dim, 1>::getHull() const
{
   return hull.get();
}

template<uint Dim>
Mesh<Dim, 2>::Mesh(vector<Matrix<double, Dim, 1>>* points) : Mesh<Dim, 1>(points)
{
   faces_owner = make_unique<vector<unique_ptr<Face<Dim>>>>();
   faces = faces_owner.get();
   ehash2id_owner = make_unique<unordered_map<ullong, size_t>>();
   ehash2id = ehash2id_owner.get();
   hull = make_unique<Mesh<Dim, 1>>(this);
}

template<uint Dim>
Face<Dim>* Mesh<Dim, 2>::getFace(size_t idx) const
{
   if (idx < reffaces.size())
      return (*faces)[reffaces[idx]].get();
   return nullptr;
}

template<uint Dim>
Face<Dim>* Mesh<Dim, 2>::getFaceByID(size_t id) const
{
   if (!binary_search(reffaces.begin(), reffaces.end(), id))
      return nullptr;
   if (id < faces->size())
      return (*faces)[id].get();
   return nullptr;
}

template<uint Dim>
Face<Dim>* Mesh<Dim, 2>::getFace(size_t eid1, size_t eid2, size_t eid3) const
{
   auto it = ehash2id->find(trihash(make_tuple(eid1, eid2, eid3)));
   if (it != ehash2id->end())
      return getFace(it->second);
   return nullptr;
}

template<uint Dim>
Face<Dim> *Mesh<Dim, 2>::getOrCreateFace(size_t eid1, size_t eid2, size_t eid3)
{
   Face<Dim>* face = nullptr;
   const ullong key = trihash(make_tuple(eid1, eid2, eid3));
   auto it = ehash2id->find(key);
   if (it == ehash2id->end()) {
      Edge<Dim>* e1 = Mesh<Dim, 1>::getOrCreateEdgeByID(eid1);
      if (e1 == nullptr)
         throw out_of_range("Edge index 1 out of range");
      Edge<Dim>* e2 = Mesh<Dim, 1>::getOrCreateEdgeByID(eid2);
      if (e2 == nullptr)
         throw out_of_range("Edge index 2 out of range");
      Edge<Dim>* e3 = Mesh<Dim, 1>::getOrCreateEdgeByID(eid3);
      if (e3 == nullptr)
         throw out_of_range("Edge index 3 out of range");
      const size_t id = faces->size();
      faces->push_back(make_unique<Face<Dim>>(Face<Dim>({e1, e2, e3}, id)));
      face = faces->back().get();
      (*ehash2id)[key] = id;
   } else
      face = (*faces)[it->second].get();
   const size_t id = face->getID();
   if (!binary_search(reffaces.begin(), reffaces.end(), id))
      Mesh<Dim, 0>::insertSorted(reffaces, id);
   return face;
}

template<uint Dim>
Face<Dim> *Mesh<Dim, 2>::getOrCreateFace(Edge<Dim>* e1, Edge<Dim>* e2, Edge<Dim>* e3)
{
   Face<Dim>* face = nullptr;
   const ullong key = trihash(make_tuple(e1->getID(), e2->getID(), e3->getID()));
   auto it = ehash2id->find(key);
   if (it == ehash2id->end()) {
      const size_t id = faces->size();
      faces->push_back(make_unique<Face<Dim>>(Face<Dim>({e1, e2, e3}, id)));
      face = faces->back().get();
      (*ehash2id)[key] = id;
   } else
      face = (*faces)[it->second].get();
   const size_t id = face->getID();
   if (!binary_search(reffaces.begin(), reffaces.end(), id))
      Mesh<Dim, 0>::insertSorted(reffaces, id);
   return face;
}

template<uint Dim>
size_t Mesh<Dim, 2>::getNumFaces() const
{
   return reffaces.size();
}

template<uint Dim>
Mesh<Dim, 1>* Mesh<Dim, 2>::getHull() const
{
   return hull.get();
}

Mesh<3, 3>::Mesh(vector<Matrix<double, 3, 1>>* points) : Mesh<3, 2>(points)
{
}

Cell* Mesh<3, 3>::getCell(size_t idx) const
{
   if (idx >= cells.size()) {
      cout << "Mesh3D getCell: Index out of bounds" << endl;
      cout.flush();
      throw out_of_range("Cell index out of range");
   }
   return cells[idx].get();
}

size_t Mesh<3, 3>::getNumCells() const
{
   return cells.size();
}

template class Mesh<0, 0>;
template class Mesh<1, 0>;
template class Mesh<2, 0>;
template class Mesh<3, 0>;
template class Mesh<1, 1>;
template class Mesh<2, 1>;
template class Mesh<2, 2>;
template class Mesh<3, 1>;
template class Mesh<3, 2>;
template class Mesh<3, 3>;