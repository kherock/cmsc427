#ifndef __MESH_HPP__

#include <QtGui>
#include <QtOpenGL>

#include <iostream>
using namespace std;


// Material specification for phong shading.
struct Material {
  Material() { 
    // Default material values. 
    Ns = 100; is_texture = false; map_Kd = NULL;
    Kd = QVector3D(0.8f, 0.8f, 0.8f);
    Ks = QVector3D(0.95f, 0.95f, 0.95f);
    Ka = QVector3D(0.1f, 0.1f, 0.1f);
  }
  QString name;
  QVector3D Ka, Kd, Ks;
  float Ns;
  bool is_texture;
  QImage map_Kd_img; // NOTE: mirrored image is stored.
  QOpenGLTexture *map_Kd;
};


// Ray with origin o a given direction d. mint specifiy the min and max t range of the ray.
struct Ray { QVector3D o, d; float mint, maxt; };

// Intersect result from AABB-ray intersection.
struct IsectAABB {  float tNear, tFar; };

// Axis aligned bounding box (AABB) with a ray intersection function.
struct AABB {
  QVector3D pMax, pMin; // max and min coordinates that specifiy corners of AABB.

  // Ray - AABB intersection.
  bool intersect(IsectAABB &isect, const Ray &ray) const {
    float t0 = ray.mint, t1 = ray.maxt;
    for (int i = 0; i < 3; ++i) {
      // Update interval for ith bounding box slab
      float invRayDir = 1.f / ray.d[i];
      float tNear = (pMin[i] - ray.o[i]) * invRayDir;
      float tFar  = (pMax[i] - ray.o[i]) * invRayDir;
      // Update parametric interval from slab intersection $t$s
      if (tNear > tFar) swap(tNear, tFar);
      t0 = tNear > t0 ? tNear : t0;
      t1 = tFar  < t1 ? tFar  : t1;
      if (t0 > t1) return false;
    }
    isect.tNear = t0;  isect.tFar = t1;
    return true;
  }
  // Returns surface area of bounding box.
  float SurfaceArea() {
    QVector3D d = pMax - pMin;  return 2.f * ( d.x() * d.y() + d.x() * d.z() + d.y() * d.z() );
  }
  // Centroid of bounding box.
  QVector3D Centroid() {  return .5f * pMin + .5f * pMax;  }
};


// Result of Ray-Triangl Intersection.
struct IsectTri {
  float b[3];  // barycentric coordiantes of intersection
  float t, eps; // t position and corresponding shadow/refract/reflect ray epsilon
  long tri_idx; // Index of triangle in triangle buffer.
};

// Triangle with vertex normals and a pointer/index to a material.
struct Triangle {
  QVector3D v[3], n[3];
  QVector2D uv[3]; // verticies and corresponding vertex normals, and uv coordiantes.
  int mtl_idx; // material idx
  
  bool intersect(IsectTri &isect, const Ray &ray)  {
    const QVector3D &o = ray.o;
    const QVector3D &d = ray.d;
    // Get triangle vertices in _p1_, _p2_, and _p3_
    const QVector3D &p1 = v[0];
    const QVector3D &p2 = v[1];
    const QVector3D &p3 = v[2];

    QVector3D e1 = p2 - p1, e2 = p3 - p1;
    QVector3D s1 = QVector3D::crossProduct(d, e2);
    float divisor = QVector3D::dotProduct(s1, e1);
    
    //if(divisor > -1e-6f && divisor < 1e-6f) return false;
    if(divisor == 0) return false;
    float invDivisor = 1.f / divisor;
    
    // Compute first barycentric coordinate
    QVector3D s = o - p1;
    float b1 = QVector3D::dotProduct(s, s1) * invDivisor;
    if (b1 < 0. || b1 > 1.) return false;

    // Compute second barycentric coordinate
    QVector3D s2 = QVector3D::crossProduct(s, e1);
    float b2 = QVector3D::dotProduct(d, s2) * invDivisor;
    if (b2 < 0. || b1 + b2 > 1.)  return false;

    // Compute _t_ to intersection point
    float t = QVector3D::dotProduct(e2, s2) * invDivisor;
    
    if (t < ray.mint || t > ray.maxt) return false;

    float b0 = 1 - b1 - b2; // Compute 3rd Barycentric coordinate

    // Svae intersection data. 
    isect.b[0] = b0; isect.b[1] = b1;  isect.b[2] = b2;
    isect.t = t; isect.eps = 1e-3f * t;
    return true;
  }


  float maxd(int d) const { return max(v[0][d], max(v[1][d], v[2][d])); }
  float mind(int d) const { return min(v[0][d], min(v[1][d], v[2][d])); }  

  // Returns bounding box around triangle.
  AABB bbox() const {
    AABB box;
    box.pMax = QVector3D(maxd(0), maxd(1), maxd(2));
    box.pMin = QVector3D(mind(0), mind(1), mind(2));    
    return box;
  }
};


// Tagged union representing a bounding volume heirarchy node.
// http://en.cppreference.com/w/cpp/language/union
struct bvhnode {
  AABB box;
  enum nodetype { SPLIT, LEAF } type;
  union { 
    bvhnode *children[2]; // children nodes.
    long ival[2];
    // only valid for LEAF type nodes, interval in triangle array
    // contained by this BVH node
  } data;
};

void bvh_delete(bvhnode *node);
 
struct Mesh_Face {
  Mesh_Face() {
    vert[0] = vert[1] = vert[2] = -1;
    vnidx[0] = vnidx[1] = vnidx[2] = -1;
    mtl_idx = -1;}
  Mesh_Face(long v0, long v1, long v2, 
      long n0, long n1, long n2,
      long t0, long t1, long t2,      
      long mtl_idx_set = -1) { 
    vert[0] = v0; vert[1] = v1; vert[2] = v2;
    mtl_idx = mtl_idx_set;
    vnidx[0] = n0; vnidx[1] = n1; vnidx[2] = n2;    
    vt[0] = t0; vt[1] = t1; vt[2] = t2;
  }
  Mesh_Face(long v0, long v1, long v2, long mtl_idx_set) { 
    vert[0] = v0; vert[1] = v1; vert[2] = v2;
    mtl_idx = mtl_idx_set;
    vt[0] = vt[1] = vt[2] = -1;
  }
  QVector3D normal; 
  long vert[3]; // indices (in the vertex array) of all vertices (mesh_vertex)
  long vnidx[3]; // indices (in the normal array)
  long vt[3]; // indicies in texture coorindate array
  long mtl_idx; // Material associated wiht mesh face.
};

struct Mesh {
  Mesh() {
    bvhroot = NULL;
  }
  ~Mesh() {
    if(bvhroot != NULL) bvh_delete(bvhroot);
    for(size_t i = 0; i < materials.size(); i++) {  if(materials[i].map_Kd != NULL) delete materials[i].map_Kd;   }
  }

  vector<QVector3D> vertices; // List of shared verticies.
  vector< vector<long> > facelist; // Adjacent face list for each vertex
  vector<QVector2D> texCoords; // Texture coordinates for each vertex.
  vector<QVector3D> normals; // Vertex normals.
  vector<Mesh_Face> faces; // Mesh faces. 
  vector<Triangle> triangles;
  
  vector<Material> materials;
  vector<int> mat_N_faces;   // For each material VBOs.
  vector<int> mat_offset;   // For each material VBOs.

  QOpenGLBuffer vertexBuffer, normalBuffer, texCoordBuffer;

  bvhnode *bvhroot;
  
  bool load_obj(QString filename, QString dir);
  bool load_mtl(QString filename, QString dir);

  void rebuild_adj();
  void compute_face_normals();
  void compute_vertex_normals();
  void storeVBO();

  void storeTri();
  void storeBVH(bool use_SAH);
  void clearBVH();
  
  void recenter();
  void get_AABB(QVector3D &maxPoint, QVector3D &minPoint);

  void add_face(const vector<long> &cur_vert, int mtl_idx);
  void add_face(int v0, int v1, int v2, int v3, int mtl_idx = 0);
  void add_face(const vector<long> &cur_vert, const vector<long> &cur_vt, const vector<long> &cur_vn, int mtl_idx);

  bool check_intersect(bool useBVH, long &aabb_cnt, long &tri_cnt, int &mtl_idx, QVector3D &pos, QVector3D &norm, Ray &ray);
};


#endif // __MESH_HPP__
