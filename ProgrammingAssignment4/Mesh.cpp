#include <iostream>
#include <fstream>
#include <iomanip>

#include "Mesh.hpp"

using namespace std;

void Mesh::add_face(vector<int> &cur_vert, vector<int> &cur_vt, int mtl_idx, int group_idx) {
  if(cur_vert.size() > 3) {
    // If number of edges in face is greater than 3,
    // decompose into triangles as a triangle fan.
    int v0 = cur_vert[0], v1 = cur_vert[1], v2 = cur_vert[2];
    int t0 = cur_vt[0], t1 = cur_vt[1], t2 = cur_vt[2];

    faces.push_back(Mesh_Face(v0, v1, v2, 
			      t0, t2, t2, mtl_idx, group_idx));
    // First face
    for( size_t i = 3; i < cur_vert.size(); i++ ) {
      v1 = v2; v2 = cur_vert[i];
      t1 = t2; t2 = cur_vt[i];
      faces.push_back(Mesh_Face(v0, v1, v2, t0, t1, t2, mtl_idx, group_idx));
    }
  }
  else if(cur_vert.size() == 3) { 
    faces.push_back(Mesh_Face(cur_vert[0], cur_vert[1], cur_vert[2], 
			      cur_vt[0], cur_vt[1], cur_vt[2], 
			      mtl_idx, group_idx));
  }
}

bool Mesh::load_obj(QString filename, QString dir) {
  QFile objfile(filename);
  if (!objfile.open(QIODevice::ReadOnly | QIODevice::Text)) { 
    return false; //error
  }
  QTextStream in(&objfile);
  long face_cnt = 0;

  long group_idx = 0;
  Mesh_Group default_group;
  default_group.name = "default";
  groups.push_back(default_group);

  // Material index 0 is the default material.
  vector<Mesh_Material> materials;
  long mtl_idx = 0;
  Mesh_Material default_mat;
  materials.push_back(default_mat);
  
  while (!in.atEnd()) {
    QString line = in.readLine(); 
    line = line.trimmed();
    line = line.replace("\t", " ");

    QStringList tokens = line.trimmed().split(' ', QString::SkipEmptyParts);
    if(tokens.size() == 0) continue;

    // Load vertex tokens.
    if(tokens[0] == "v") {
      if(tokens.size() < 4) return false; // eror
      float x = tokens[1].toFloat();
      float y = tokens[2].toFloat();
      float z = tokens[3].toFloat();
      vertices.push_back(QVector3D(x,y,z));
    }
    // Load texture coordinates. 
    if(tokens[0] == "vt") {
      if(tokens.size() < 3) return false;
      float u = tokens[1].toFloat();
      float v = tokens[2].toFloat();
      texCoords.push_back(QVector2D(u,v));
    }
    // Load faces with their indicies pointing to verticies and
    // texture coordinates.
    if(tokens[0] == "f") {
      vector<int> cur_vert, cur_vt;
      for(int i = 1; i < tokens.size(); i++) {
	QStringList indexes = tokens[i].split("/");
	if(indexes.size() >= 1) {
	  if(indexes[0].toLong() < 0) {  cur_vert.push_back(vertices.size() + indexes[0].toLong()); }
	  else { cur_vert.push_back(indexes[0].toLong() - 1); }
	  if(indexes.size() >= 2) {
	    if(indexes[1] == "") cur_vt.push_back(-1);
	    else {
	      if(indexes[1].toLong() < 0) { cur_vt.push_back(texCoords.size() + indexes[1].toLong()); }
	      else { cur_vt.push_back(indexes[1].toLong() - 1); }
	    }
	  }
	  else {
	    cur_vt.push_back(-1);
	  }
	}
      }
      // NOTE: Group and material index are passed along with this face.
      face_cnt++;
      add_face(cur_vert, cur_vt, mtl_idx, group_idx);
    }
    // Load material data.
    if(tokens[0] == "mtllib") {
      QStringList tokens = line.split(' ', QString::SkipEmptyParts);
      if(tokens.size() < 2) continue;
      QString mtllib_file = tokens[1];
      mtllib_file = dir + "/" + mtllib_file;
      bool ret = load_mtl(materials, mtllib_file, dir);
      if(!ret) return false;
    }
    // Set material index for the faces that were loaded.
    if(tokens[0] == "usemtl") {
      if(tokens.size() < 2) return false;
      QString mtl_name = tokens[1];
      for(long idx = 0; idx < (long)materials.size(); idx++) {
	if(materials[idx].name == mtl_name) {
	  cout << mtl_name.toStdString() << endl;
	  mtl_idx = idx;
	  break;
	}
      }
    }
    // Load in a group.
    if(tokens[0] == "g") {
      string cur_group_name;
      // Use integer to specify group if name is not given.
      if(tokens.size() < 2) cur_group_name = std::to_string(groups.size()); 
      else cur_group_name = tokens[1].toStdString();
      // Check if group was found, and set grou pindex accordingly.
      bool group_found = false;
      for(long idx = 0; idx < (long)groups.size(); idx++) {
	if(groups[idx].name == cur_group_name) {
	  group_idx = idx;
	  group_found = true;
	  break;
	}
      }
      // Otherwise this is a new group of verticies/normals/texture coordiantes.
      if(!group_found) {
	group_idx = groups.size();
	Mesh_Group cur_group;
	cur_group.name = cur_group_name;
	groups.push_back(cur_group);
      }
    }
  }
  cout << "materials.size()=" << materials.size() << endl;
  cout << "face_cnt=" << face_cnt << endl;
  cout << "faces.size()=" << faces.size() << endl;
  cout << "vertices.size()=" << vertices.size() << endl;
  cout << "texCoords.size()=" << texCoords.size() << endl;
  rebuild_adj();
  // NOTE: The above impleme
  compute_face_normals();
  // Note that we do not load the normals fo
  if(normals.size() == 0) compute_vertex_normals();
  cout << "normals.size()=" << normals.size() << endl;
  // Make sure each group has the same number of materials.
  for(size_t g = 0; g < groups.size(); g++) {
    groups[g].materials = materials;
  }
  return true;
}


bool Mesh::load_mtl(vector<Mesh_Material> &materials, QString filename, QString dir) {
  QFile objfile(filename);
  if (!objfile.open(QIODevice::ReadOnly | QIODevice::Text)) { 
    return false; //error
  }
  QTextStream in(&objfile);
  Mesh_Material mat;
  while (!in.atEnd()) {
    QString line = in.readLine(); 
    line = line.trimmed();
    QStringList tokens = line.split(' ', QString::SkipEmptyParts);
    if(tokens.size() == 0) continue;
    if(tokens[0] == "newmtl") {
      if(tokens.size() < 2) return false;
      if(mat.name != "") {
	materials.push_back(mat);
      }
      mat.name = tokens[1];
    }
    if(tokens[0] == "Ka") {
      if(tokens.size() < 4) return false;
      mat.Ka[0] = tokens[1].toFloat();
      mat.Ka[1] = tokens[2].toFloat();
      mat.Ka[2] = tokens[3].toFloat();
    }
    if(tokens[0] == "Kd") {
      if(tokens.size() < 4) return false;
      mat.Kd[0] = tokens[1].toFloat();
      mat.Kd[1] = tokens[2].toFloat();
      mat.Kd[2] = tokens[3].toFloat();
    }
    if(tokens[0] == "Ks") {
      if(tokens.size() < 4) return false;
      mat.Ks[0] = tokens[1].toFloat();
      mat.Ks[1] = tokens[2].toFloat();
      mat.Ks[2] = tokens[3].toFloat();
    }
    if(tokens[0] == "Ns") {
      if(tokens.size() < 2) return false;
      mat.Ns = tokens[1].toFloat();
    }
    if(tokens[0] == "map_Kd") {
      if(tokens.size() < 2) return false;
      QString texture_file = tokens[1];
      texture_file = dir + "/" + texture_file;
      cout << "loading " << texture_file.toStdString() << endl;
      mat.map_Kd = std::make_shared<QOpenGLTexture>(QImage(texture_file).mirrored());
      mat.map_Kd->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::LinearMipMapLinear);
      mat.is_texture = true;
      cout << "*DONE*" << endl;
    }
  }
  if(mat.name != "") materials.push_back(mat);
  return true;
}

void Mesh::rebuild_adj() {
  // Compute how much space is needed for the face list on each vertex.
  vector<int> fadjcnt(vertices.size(), 0);
  for(size_t fidx = 0; fidx < faces.size(); fidx++) {
    for(int i = 0; i < 3; i++) fadjcnt.at(faces[fidx].vert[i])++;
  }

  for(size_t vidx = 0; vidx < vertices.size(); vidx++) 
    if(fadjcnt[vidx] == 0) { cerr << "no adjacent face for vertex" << endl;  }

  // Reserve space for each face list.
  facelist.resize(vertices.size());

  // Clear, if any are present.
  for(size_t vidx = 0; vidx < facelist.size(); vidx++) facelist[vidx].clear();
  for(size_t vidx = 0; vidx < facelist.size(); vidx++) facelist[vidx].reserve(fadjcnt[vidx]); 

  // Populate each face list.
  for(int fidx = 0; fidx < (int)faces.size(); fidx++) {
    for(int i = 0; i < 3; i++) facelist[faces[fidx].vert[i]].push_back(fidx);
  }
}


inline QVector3D ComputeNormal(QVector3D &p0, QVector3D &p1, QVector3D &p2) {
  QVector3D v1 = p1 - p0, v2 = p2 - p0;
  QVector3D normal = QVector3D::crossProduct(v1, v2);
  normal.normalize();
  return normal;
}
// Compute face normals using vertex indicies.
void Mesh::compute_face_normals() {
  for(int fidx = 0; fidx < (int)faces.size(); fidx++) {
    Mesh_Face &F = faces[fidx];
    QVector3D &v0 = vertices[F.vert[0]];
    QVector3D &v1 = vertices[F.vert[1]];
    QVector3D &v2 = vertices[F.vert[2]];
    faces[fidx].normal = ComputeNormal(v0, v1, v2);
  }
}


// Computes vertex normal as the mean of the neighboring face normals.
void Mesh::compute_vertex_normals() {
  // Compute vertex normals. 
  normals.reserve(vertices.size());
  normals.resize(vertices.size());
  for(size_t vidx = 0; vidx < vertices.size(); vidx++) {
    vector<long> &adj = facelist[vidx]; // Get face adjacencies.
    QVector3D vnormal(0,0,0); 
    for(size_t a = 0; a < adj.size(); a++) vnormal += faces[adj[a]].normal;
    float Z = (float)adj.size();
    normals[vidx] = (1.0f / Z) * vnormal;
    normals[vidx].normalize();
  }
}

void Mesh::storeVBO_groups() {
  for(long group_idx = 0; group_idx < (long)groups.size(); ++group_idx) {
    for(long mtl_idx = 0; mtl_idx < (long)groups[group_idx].materials.size(); mtl_idx++) {
      // Get face indexes that match current group and current material.
      vector<long> mtl_faces;
      for(long f = 0; f < (long)faces.size(); f++) {
	if(faces[f].mtl_idx == mtl_idx && faces[f].group_idx == group_idx)
	  mtl_faces.push_back(f);
      }
      vector<QVector3D> tri_vert;
      vector<QVector3D> tri_norm;
      vector<QVector2D> tri_tex;    
      // Collect triangles + normals + vertex coords for those matching
      // the current material and the current group.
      for(long mf = 0; mf < (long)mtl_faces.size(); mf++) {
	long f = mtl_faces[mf];
	tri_vert.push_back(vertices.at(faces[f].vert[0]));
	tri_vert.push_back(vertices.at(faces[f].vert[1]));
	tri_vert.push_back(vertices.at(faces[f].vert[2]));

	tri_norm.push_back(normals.at(faces[f].vert[0]));
	tri_norm.push_back(normals.at(faces[f].vert[1]));
	tri_norm.push_back(normals.at(faces[f].vert[2]));

	if(faces[f].vt[0] >= 0) tri_tex.push_back(texCoords.at(faces[f].vt[0]));
	else tri_tex.push_back(QVector2D(0,0));

	if(faces[f].vt[1] >= 0) tri_tex.push_back(texCoords.at(faces[f].vt[1]));
	else tri_tex.push_back(QVector2D(0,0));
      
	if(faces[f].vt[2] >= 0) tri_tex.push_back(texCoords.at(faces[f].vt[2]));
	else tri_tex.push_back(QVector2D(0,0));      
      }
      // For the current group and the current material in that group, fill the
      // vertex, normal, and texture buffers. 
      groups[group_idx].materials[mtl_idx].fill_buffers(tri_vert, tri_norm, tri_tex);
    }
  }
}

