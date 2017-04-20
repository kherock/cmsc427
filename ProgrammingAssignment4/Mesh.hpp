#ifndef __MESH_HPP__

#include <QtGui>
#include <QtOpenGL>

#include <iostream>
#include <memory>

using namespace std;

// Material specification for phong shading.
struct Mesh_Material {
  Mesh_Material() { 
    // Default material values. 
    Ns = 100;
    is_texture = false; map_Kd = NULL;
    Kd = QVector3D(0.8, 0.8, 0.8);
    Ks = QVector3D(0.95, 0.95, 0.95);
    Ka = QVector3D(0.1, 0.1, 0.1);
    vertexBuffer = normalBuffer = texCoordBuffer = NULL;
  }
  QString name;
  QVector3D Ka, Kd, Ks;
  float Ns;
  bool is_texture;
  std::shared_ptr<QOpenGLTexture> map_Kd; // pointer to OpenGLTexture data
  std::shared_ptr<QOpenGLBuffer> vertexBuffer, normalBuffer, texCoordBuffer;
  int n_triangles = 0;

  // Fill buffers with triangle, normal, and texture coordinate data.
  bool fill_buffers(vector<QVector3D> &tri_vert, vector<QVector3D> &tri_norm, vector<QVector2D> &tri_tex) {
    if(tri_vert.size() != tri_norm.size()) return false;
    if(tri_vert.size() != tri_tex.size())  return false;

    n_triangles = tri_vert.size() / 3;

    vertexBuffer = std::make_shared<QOpenGLBuffer>();
    normalBuffer = std::make_shared<QOpenGLBuffer>();
    texCoordBuffer = std::make_shared<QOpenGLBuffer>();    

    if(vertexBuffer->isCreated()) vertexBuffer->destroy();
    vertexBuffer->create();
    vertexBuffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
    vertexBuffer->bind();
    vertexBuffer->allocate(tri_vert.data() , sizeof( QVector3D ) * tri_vert.size());

    if(normalBuffer->isCreated()) normalBuffer->destroy();
    normalBuffer->create();
    normalBuffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
    normalBuffer->bind();
    normalBuffer->allocate(tri_norm.data() , sizeof( QVector3D ) * tri_norm.size());    

    if(texCoordBuffer->isCreated()) texCoordBuffer->destroy();
    texCoordBuffer->create();
    texCoordBuffer->setUsagePattern( QOpenGLBuffer::StaticDraw );
    texCoordBuffer->bind();
    texCoordBuffer->allocate(tri_tex.data() , sizeof( QVector2D ) * tri_tex.size());    

    return true;
  }
  
};

struct Mesh_Face {
  Mesh_Face() {
    vert[0] = vert[1] = vert[2] = -1;
    vt[0] = vt[1] = vt[2] = -1;
    mtl_idx = -1; group_idx = -1;
  }
  Mesh_Face(long v0, long v1, long v2,
	    long t0, long t1, long t2,
	    long mtl_idx_set, long group_idx_set ) { 
    vert[0] = v0; vert[1] = v1; vert[2] = v2;
    vt[0] = t0; vt[1] = t1; vt[2] = t2;
    mtl_idx = mtl_idx_set;
    group_idx = group_idx_set;
  }
  QVector3D normal; 
  long vert[3]; // indices (in the vertex array) of all vertices (mesh_vertex)
  long vt[3];   // indicies in texture array.
  long mtl_idx; // map to index of materials in vector<Materials> materials;
  long group_idx; // group to which the face belongs, map to index in vector<string> groups;
};

struct Mesh_Group {
  string name;
  vector<Mesh_Material> materials; // Each Mesh_Group has the same number of Mesh_Materials.
};

struct Mesh {
  Mesh() {  }
  virtual ~Mesh() { }

  // Transformation to apply to entire model.
  QQuaternion model_rotation;
  QVector3D model_translate;
  float model_sx = 1, model_sy = 1, model_sz = 1;
  // ----
  
  vector<QVector3D> vertices; // List of shared verticies.
  vector< vector<long> > facelist; // Adjacent face list for each vertex
  vector<QVector2D> texCoords; // Texture coordinates for each vertex.
  vector<QVector3D> normals; // Vertex normals.
  vector<Mesh_Face> faces; // Mesh faces. 

  vector<Mesh_Group> groups; // Mesh groups.

  bool load_obj(QString filename, QString dir);
  bool load_mtl(vector<Mesh_Material> &materials, QString filename, QString dir);

  void rebuild_adj();
  void compute_face_normals();
  void compute_vertex_normals();
  void storeVBO_groups();
  void add_face(vector<int> &cur_vert, vector<int> &cur_vt, int mtl_idx, int group_idx);
};


#endif // __MESH_HPP__
