#include <fstream>
#include <iomanip>
#include <iostream>

#include "Mesh.hpp"

using namespace std;

// Adds a two-way edge if v1 isn't marked as a neighbor to v0
void Mesh::add_unique_edge(int v0, int v1) {
  for (int i = 0; i < vertices[v0].edges.size(); i++) {
    if (vertices[v0].edges[i] == v1) return;
  }
  vertices[v0].edges.push_back(v1);
  vertices[v1].edges.push_back(v0);
}

// Splits an edge and returns the index of the vertex created between them
int Mesh::split_edge(int v0, int v1) {
  Vertex midpoint = Vertex((vertices[v0].v + vertices[v1].v) / 2);
  int idx = vertices.size();
  for (int i = 0; i < vertices[v0].edges.size(); i++) {
    if (vertices[v0].edges[i] == v1) {
      vertices[v0].edges[i] = idx;
      break;
    }
  }
  for (int i = 0; i < vertices[v1].edges.size(); i++) {
    if (vertices[v1].edges[i] == v0) {
      vertices[v1].edges[i] = idx;
      break;
    }
  }
  midpoint.edges.push_back(v0);
  midpoint.edges.push_back(v1);
  // Add the two faces that both vertices share
  for (int i = 0; i < vertices[v0].faces.size(); i++) {
    for (int j = 0; j < vertices[v1].faces.size(); j++) {
      if (vertices[v0].faces[i] == vertices[v1].faces[j]) {
        midpoint.faces.push_back(vertices[v0].faces[i]);
      }
    }
  }
  vertices.push_back(midpoint);
  return idx;
}

void Mesh::add_face(const vector<int> &cur_vert) {
  int v0 = cur_vert[0], v1 = cur_vert[1], v2 = cur_vert[2];

  add_unique_edge(v0, v1);
  add_unique_edge(v0, v2);
  add_unique_edge(v1, v2);
  vertices[v0].faces.push_back(faces.size());
  vertices[v1].faces.push_back(faces.size());
  vertices[v2].faces.push_back(faces.size());
  faces.push_back(Mesh_Face(v0, v1, v2)); // First face

  if (cur_vert.size() > 3) {
    // If number of edges in face is greater than 3,
    // decompose into triangles as a triangle fan.

    // all subsequent faces
    for (size_t i = 3; i < cur_vert.size(); i++) {
      v1 = v2;
      v2 = cur_vert[i];
      add_unique_edge(v0, v1);
      add_unique_edge(v0, v2);
      add_unique_edge(v1, v2);
      vertices[v0].faces.push_back(faces.size());
      vertices[v1].faces.push_back(faces.size());
      vertices[v2].faces.push_back(faces.size());
      faces.push_back(Mesh_Face(v0, v1, v2));
    }
  }
}

bool Mesh::load_obj(QString filename) {
  QFile objfile(filename);
  if (!objfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false; // error
  }
  QTextStream in(&objfile);
  long face_cnt = 0;

  while (!in.atEnd()) {
    QString line = in.readLine();
    line = line.trimmed();
    line = line.replace("\t", " ");

    QStringList tokens = line.trimmed().split(' ', QString::SkipEmptyParts);
    if (tokens.size() == 0) continue;

    if (tokens[0] == "v") {
      if (tokens.size() < 4) return false; // eror
      float x = tokens[1].toFloat();
      float y = tokens[2].toFloat();
      float z = tokens[3].toFloat();
      vertices.push_back(Vertex(x, y, z));
    }
    if (tokens[0] == "f") {
      vector<int> cur_vert;
      for (int i = 1; i < tokens.size(); i++) {
        QStringList indexes = tokens[i].split("/");
        if (indexes.size() >= 1) {
          if (indexes[0].toLong() < 0) {
            cur_vert.push_back(vertices.size() + indexes[0].toLong());
          } else {
            cur_vert.push_back(indexes[0].toLong() - 1);
          }
        }
      }
      if (cur_vert.size() >= 3) {
        face_cnt++;
        add_face(cur_vert);
      }
    }
  }
  cout << "face_cnt=" << face_cnt << endl;
  cout << "faces.size()=" << faces.size() << endl;
  cout << "vertices.size()=" << vertices.size() << endl;

  recenter();
  return true;
}

void Mesh::recenter() {
  if (vertices.size() < 1) return;
  QVector3D maxPoint = vertices[0].v;
  QVector3D minPoint = vertices[0].v;

  // Find the AABB
  for (uint i = 0; i < vertices.size(); ++i) {
    QVector3D &point = vertices[i].v;
    if (point[0] > maxPoint[0]) maxPoint[0] = point[0];
    if (point[1] > maxPoint[1]) maxPoint[1] = point[1];
    if (point[2] > maxPoint[2]) maxPoint[2] = point[2];
    if (point[0] < minPoint[0]) minPoint[0] = point[0];
    if (point[1] < minPoint[1]) minPoint[1] = point[1];
    if (point[2] < minPoint[2]) minPoint[2] = point[2];
  }

  // Center of the AABB
  QVector3D center = QVector3D(
      (maxPoint[0] + minPoint[0]) / 2.0f,
      (maxPoint[1] + minPoint[1]) / 2.0f,
      (maxPoint[2] + minPoint[2]) / 2.0f
  );

  // Translate center of the AABB to the origin
  for (uint i = 0; i < vertices.size(); ++i) {
    QVector3D &point = vertices[i].v;
    point = point - center;
  }
}

void Mesh::process_example() {
  for (size_t v = 0; v < vertices.size(); v++) {
    if (vertices[v].v[0] > 0) {
      vertices[v].v[0] += 3.5;
    }
  }
}

void Mesh::storeVBO() {
  vector<QVector3D> tri_vert, tri_bary;

  for (long f = 0; f < (long)faces.size(); f++) {
    tri_vert.push_back(vertices.at(faces[f].vert[0]).v);
    tri_vert.push_back(vertices.at(faces[f].vert[1]).v);
    tri_vert.push_back(vertices.at(faces[f].vert[2]).v);

    tri_bary.push_back(QVector3D(1, 0, 0));
    tri_bary.push_back(QVector3D(0, 1, 0));
    tri_bary.push_back(QVector3D(0, 0, 1));
  }

  if (vertexBuffer.isCreated()) vertexBuffer.destroy();
  vertexBuffer.create();
  vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
  vertexBuffer.bind();
  vertexBuffer.allocate(&tri_vert[0], sizeof(QVector3D) * tri_vert.size());

  if (baryBuffer.isCreated()) baryBuffer.destroy();
  baryBuffer.create();
  baryBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
  baryBuffer.bind();
  baryBuffer.allocate(&tri_bary[0], sizeof(QVector3D) * tri_bary.size());
}

// Calculates the average edge lengths and normal for every vertex in the mesh
void Mesh::updateVertices() {
  for (int i = 0; i < vertices.size(); i++) {
    // Compute average edge length
    Vertex &v0 = vertices[i];
    for (int j = 0; j < v0.edges.size(); j++) {
      Vertex &v1 = vertices[v0.edges[j]];
      v0.avgEdgeLen = (v0.avgEdgeLen * j + (v0.v - v1.v).length()) / (j + 1);
    }

    // Compute normal, weighted by the area of each associated face
    v0.normal = QVector3D();
    for (int j = 0; j < v0.faces.size(); j++) {
      Mesh_Face &f = faces[v0.faces[j]];
      int idx = 0;
      while (f.vert[idx] != i) idx++;

      // Get the neighboring vertices that make up this face
      // The vertices are assumed to be defined in a counterclockwise order
      Vertex *v1, *v2;
      switch (idx) {
      case 0:
        v1 = &vertices[f.vert[1]];
        v2 = &vertices[f.vert[2]];
        break;
      case 1:
        v1 = &vertices[f.vert[2]];
        v2 = &vertices[f.vert[0]];
        break;
      case 2:
        v1 = &vertices[f.vert[0]];
        v2 = &vertices[f.vert[1]];
        break;
      }

      QVector3D a = v0.v - v1->v;
      QVector3D b = v0.v - v2->v;
      QVector3D c = v1->v - v2->v;

      QVector3D crossProduct = QVector3D(
          a.y() * b.z() - a.z() * b.y(),
          a.z() * b.x() - a.x() * b.z(),
          a.x() * b.y() - a.y() * b.x()
      ).normalized();
      // Use Heron's formula to get the area of the face
      float s = 0.5 * (a.length() + b.length() + c.length());
      v0.normal += crossProduct * sqrt(s * (s - a.length()) * (s - b.length()) * (s - c.length()));
    }
    v0.normal.normalize();
  }
}
