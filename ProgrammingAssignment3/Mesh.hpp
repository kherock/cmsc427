#ifndef __MESH_HPP__

#include <QtGui>
#include <QtOpenGL>

#include <iostream>
#include <map>
using namespace std;

struct Mesh_Face {
  Mesh_Face() { vert[0] = vert[1] = vert[2] = -1; }
  Mesh_Face(long v0, long v1, long v2) {
    vert[0] = v0;
    vert[1] = v1;
    vert[2] = v2;
  }
  long vert[3]; // indices (in the vertex array) of all vertices (mesh_vertex)
};

struct Vertex {
  Vertex() { v = QVector3D(); }
  Vertex(QVector3D vec) { v = vec; }
  Vertex(float x, float y, float z) { v = QVector3D(x, y, z); }

  QVector3D v;

  QVector3D normal;
  float avgEdgeLen;
  vector<long> edges;
  vector<long> faces;
};

struct Mesh {
  vector<Vertex> vertices; // List of shared verticies.
  vector<Mesh_Face> faces; // Mesh faces.
  QOpenGLBuffer vertexBuffer, baryBuffer;

  bool load_obj(QString filename);
  void storeVBO();
  void updateVertices();
  void recenter();
  void add_unique_edge(int v0, int v1);
  int split_edge(int v0, int v1);
  void add_face(const vector<int> &cur_vert);
  void process_example();
};

#endif // __MESH_HPP__
