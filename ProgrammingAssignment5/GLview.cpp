#include "GLview.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

using namespace std;

float copysign0(float x, float y) { return (y == 0.0f) ? 0 : copysign(x,y); }

void Matrix2Quaternion(QQuaternion &Q, QMatrix4x4 &M) {
  Q.setScalar(sqrt( max( 0.0f, 1 + M(0,0)  + M(1,1) + M(2,2) ) ) / 2);
  Q.setX(sqrt( max( 0.0f, 1 + M(0,0) - M(1,1) - M(2,2) ) ) / 2);
  Q.setY(sqrt( max( 0.0f, 1 - M(0,0) + M(1,1) - M(2,2) ) ) / 2);
  Q.setZ(sqrt( max( 0.0f, 1 - M(0,0) - M(1,1) + M(2,2) ) ) / 2);
  Q.setX(copysign0( Q.x(), M(2,1) - M(1,2) ));  Q.setY(copysign0( Q.y(), M(0,2) - M(2,0) ));  Q.setZ(copysign0( Q.z(), M(1,0) - M(0,1) )) ;
}

// GLView constructor. DO NOT MODIFY.
GLview::GLview(QWidget *parent)  : QOpenGLWidget(parent) {
  scaleFlag = translateFlag = rotateFlag = false; lastPosFlag = false;
  mesh = NULL;

  startTimer(20, Qt::PreciseTimer);
  
  elapsed_time.start(); elapsed_time.invalidate();  
  timeAccumulator = 0; totalTime = 0;

  lightMotionFlag = false;
}  

// Load object file. DO NOT MODIFY.
bool GLview::LoadOBJFile(const QString file, const QString path) {
  makeCurrent();
  Mesh *newmesh = new Mesh;
  if(!newmesh->load_obj(file, path)) { delete newmesh;  return false;  }
  if(mesh != NULL) delete mesh;
  mesh = newmesh;
  
  mesh->storeVBO();

  // Update VBO associated with texture shader. 
  texture_shader.bind();

  mesh->vertexBuffer.bind();
  texture_shader.setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 3 );
  texture_shader.enableAttributeArray( "VertexPosition" );

  mesh->normalBuffer.bind();
  texture_shader.setAttributeBuffer( "VertexNormal", GL_FLOAT, 0, 3 );
  texture_shader.enableAttributeArray( "VertexNormal" );      
    
  mesh->texCoordBuffer.bind();
  texture_shader.setAttributeBuffer( "VertexTexCoord", GL_FLOAT, 0, 2 );
  texture_shader.enableAttributeArray( "VertexTexCoord" );

  doneCurrent();
  return true;
}

// Set default GL parameters. 
void GLview::initializeGL() {
  initializeOpenGLFunctions();
  vao.create(); if (vao.isCreated()) vao.bind();

  glClearColor( 0.15f, 0.15f, 0.15f, 1.0f );   // Set the clear color to black
  glEnable(GL_DEPTH_TEST);    // Enable depth buffer

  // Prepare a complete shader program...
  if ( !prepareShaderProgram(texture_shader,  ":/texture.vsh", ":/texture.fsh" ) ) return;

  // Set default lighting parameters. 
  LightDirection = QVector3D(2,2, 1);  LightIntensity = QVector3D(1,1,1);

  // Initialize default camera parameters
  yfov = 55;
  neardist = 0.1f; fardist = 30;
  eye = QVector3D(-2,2,2); lookCenter = QVector3D(0,0,0); lookUp = QVector3D(0,0,1);

  QMatrix4x4 view; view.lookAt(eye, lookCenter, lookUp);
  Matrix2Quaternion(camrot, view);
}

// Set the viewport to window dimensions. DO NOT MODIFY.
void GLview::resizeGL( int w, int h ) {
  resize_width = w; resize_height = h;
  glViewport( 0, 0, w, qMax( h, 1 ) );
}

void GLview::initLightCameraGL() {
  QMatrix4x4 model, view, projection;

  view.rotate(camrot); view.translate(-eye);
  projection.perspective(yfov, (float)width() / (float)height(), neardist, fardist);

  QMatrix4x4 model_view = view * model;
  QMatrix3x3 normal_matrix = model_view.normalMatrix();
  QMatrix4x4 MVP = projection * model_view;

  texture_shader.bind();
  texture_shader.setUniformValue("ModelViewMatrix", model_view);
  texture_shader.setUniformValue("NormalMatrix", normal_matrix);
  texture_shader.setUniformValue("MVP", MVP);
  texture_shader.setUniformValue("LightIntensity", LightIntensity);
  texture_shader.setUniformValue("LightDirection", view * QVector4D(LightDirection, 0));
}

void GLview::paintGL() {
  initLightCameraGL(); // Update lighting and camara position.

  // Clear the buffer with the current clearing color
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if(mesh == NULL) return;

  texture_shader.bind();
  for(long mtl_idx = 0; mtl_idx < (long)mesh->materials.size(); mtl_idx++) {
    if(mesh->mat_N_faces[mtl_idx] == 0) continue;
    texture_shader.setUniformValue("Kd", mesh->materials[mtl_idx].Kd);
    texture_shader.setUniformValue("Ks", mesh->materials[mtl_idx].Ks);
    texture_shader.setUniformValue("Ka", mesh->materials[mtl_idx].Ka);
    texture_shader.setUniformValue("Shininess", mesh->materials[mtl_idx].Ns);
    if(mesh->materials[mtl_idx].map_Kd != NULL) {
      texture_shader.setUniformValue("useTexture", true);
      glActiveTexture(GL_TEXTURE0);  
      mesh->materials[mtl_idx].map_Kd->bind(0); 
      texture_shader.setUniformValue("Tex1", (int)0);
    }
    else texture_shader.setUniformValue("useTexture", false);
    glDrawArrays( GL_TRIANGLES, mesh->mat_offset[mtl_idx], 3*mesh->mat_N_faces[mtl_idx] );
  }

}

void GLview::keyPressGL(QKeyEvent* e) {
  switch ( e->key() ) {
  case Qt::Key_Escape:
    QCoreApplication::instance()->quit();
    break;
    // Set rotate, scale, translate modes (for track pad mainly).
  case Qt::Key_R: toggleRotate();    break;
  case Qt::Key_S: toggleScale();     break;
  case Qt::Key_T: toggleTranslate();     break;
  default: QOpenGLWidget::keyPressEvent( e );  break;
  }
}


// Compile shaders. DO NOT MODIFY.
bool GLview::prepareShaderProgram(QOpenGLShaderProgram &prep_shader, 
          const QString &vertex_file, const QString &fragment_file) {
  // First we load and compile the vertex shader.
  bool result = prep_shader.addShaderFromSourceFile( QOpenGLShader::Vertex, vertex_file );
 if ( !result ) qWarning() << prep_shader.log();

  // ...now the fragment shader...
  result = prep_shader.addShaderFromSourceFile( QOpenGLShader::Fragment, fragment_file );
  if ( !result ) qWarning() << prep_shader.log();

  // ...and finally we link them to resolve any references.
  result = prep_shader.link();
  if ( !result ) {   qWarning() << "Could not link shader program:" << prep_shader.log(); exit(1);   }
  return result;
}

// Store mouse press position. DO NOT MODIFY
void GLview::mousePressEvent(QMouseEvent *event) { 
  if(mesh == NULL) return;

  float px = event->x(), py = event->y();
  float x = 2.0 * (px + 0.5) / float(width())  - 1.0,  y = -(2.0 * (py + 0.5) / float(height()) - 1.0); 
  lastPosX = x; lastPosY = y;
  lastPosFlag = true; 
  event->accept(); 
}

// Update camera position on mouse movement. DO NOT MODIFY.
void GLview::mouseMoveEvent(QMouseEvent *event) {
  if(mesh == NULL) return;

  float px = event->x(), py = event->y();
  float x = 2.0 * (px + 0.5) / float(width())  - 1.0;
  float y = -(2.0 * (py + 0.5) / float(height()) - 1.0); 

  // Record a last position if none has been set.  
  if(!lastPosFlag) { lastPosX = x; lastPosY = y; lastPosFlag = true; return; }
  float dx = x - lastPosX, dy = y - lastPosY;
  lastPosX = x; lastPosY = y; // Remember mouse position.

  if (rotateFlag || (event->buttons() & Qt::LeftButton)) { // Rotate scene around a center point.   
    float theta_y = 2.0 * dy / M_PI * 180.0f;
    float theta_x = 2.0 * dx / M_PI * 180.0f;

    QQuaternion revQ = camrot.conjugate();
    QQuaternion newrot = QQuaternion::fromAxisAndAngle(lookUp, theta_x);
    revQ = newrot * revQ;

    QVector3D side = revQ.rotatedVector(QVector3D(1,0,0));
    QQuaternion newrot2 = QQuaternion::fromAxisAndAngle(side, theta_y);
    revQ = newrot2 * revQ;
    revQ.normalize();
    
    camrot = revQ.conjugate().normalized();    

    eye = newrot.rotatedVector(eye - lookCenter) + lookCenter;
    eye = newrot2.rotatedVector(eye - lookCenter) + lookCenter;
  } 

  if (scaleFlag || (event->buttons() & Qt::MidButton)) { // Scale the scene.
    float factor = dx + dy;
    factor = exp(2.0 * factor);
    factor = (factor - 1.0) / factor;
    QVector3D translation = (lookCenter - eye) * factor; 
    eye += translation;
  }
  if (translateFlag || (event->buttons() & Qt::RightButton)) { // Translate the scene.
    QQuaternion revQ = camrot.conjugate().normalized();
    QVector3D side = revQ.rotatedVector(QVector3D(1,0,0));
    QVector3D upVector = revQ.rotatedVector(QVector3D(0,1,0));

    float length = lookCenter.distanceToPoint(eye) * tanf(yfov * M_PI / 180.0f);
    QVector3D translation = -((side * (length * dx)) + (upVector * (length * dy) ));
    eye += translation;
    lookCenter += translation;
  }
  event->accept();
}


void GLview::updateGLview(float dt) {
  if(lightMotionFlag) {
    // Rotate the light direction about the up axis.
    QQuaternion q1 = QQuaternion::fromAxisAndAngle(lookUp, dt * 30);
    LightDirection = q1.rotatedVector(LightDirection);
  }
}

void GLview::timerEvent(QTimerEvent *){  
  if(!elapsed_time.isValid()) {   elapsed_time.restart(); return;  } // Skip first udpate.
  qint64 nanoSec = elapsed_time.nsecsElapsed();
  elapsed_time.restart();  

  double dt = 0.01;
  double frameTime = double(nanoSec) * 1e-9;

  timeAccumulator += frameTime;
  while ( timeAccumulator >= dt ) {
    updateGLview(dt);  totalTime += dt;  timeAccumulator -= dt;
  }

  update();  
}

// Ray-cast renderer is here.
bool GLview::Render(QString pngFile, bool useBVH, bool useSAH) {
  if(mesh == NULL) return true;

  auto t0 = chrono::high_resolution_clock::now();

  mesh->storeTri();
  if (useBVH) mesh->storeBVH(useSAH);

  //int img_width = width(), img_height = height();   // Get image width and height from window.
  int img_width = resize_width, img_height = resize_height;   // Get image width and height from window.
  
  QImage output(img_width, img_height, QImage::Format_RGB32);

  // Use camera quarternion to get orthonormal basis of camera.
  QQuaternion revQ = camrot.conjugate().normalized();
  QVector3D forward = revQ.rotatedVector(QVector3D(0,0,-1)).normalized();
  QVector3D side = revQ.rotatedVector(QVector3D(1,0,0)).normalized();
  QVector3D up = revQ.rotatedVector(QVector3D(0,1,0)).normalized();;

  // Use y-axis FOV to compute distance to project plane.
  float yfov_rad = yfov / 180.0f * M_PI;
  float D = 1.0 / tanf(yfov_rad/2.0); //distance to projection plane.
  float ar = float(img_width) / float(img_height); // get aspect ratio

  long aabb_cnt = 0, tri_cnt = 0;
  for (int imgy = 0; imgy < img_height; imgy++) {
    for (int imgx = 0; imgx < img_width; imgx++) {
      float red = 0, green = 0, blue = 0;
      float px = float(imgx) + 0.5;
      float py = float(imgy) + 0.5;

      float xndc = 2.0 * px / float(img_width) - 1.0;
      float yndc = -(2.0 * py / float(img_height) - 1.0);

      // Construct ray to cast from camera.
      Ray ray;
      ray.d = D * forward + up * yndc + side * ar * xndc;
      ray.mint = neardist;  ray.maxt = fardist;
      ray.d.normalize();
      ray.o = eye;

      // Check ray for intersection and get normal and position of intersection point.
      QVector3D Position, Normal;
      QVector2D UV;
      int mtl_idx = -1;
      if (mesh->check_intersect(useBVH, aabb_cnt, tri_cnt, mtl_idx, Position, Normal, UV, ray)) {
        // Phong shading.
        Material &m = mesh->materials[mtl_idx];

        QVector3D e = eye;
        QVector3D l = QVector3D(LightDirection);
        QVector3D p = Position;
        QVector3D Li = LightIntensity;

        QVector3D n = Normal.normalized();
        QVector3D s = l.normalized(); 
        QVector3D v = (e - p).normalized();   

        QVector3D r = -s + 2 * QVector3D::dotProduct(s, n) * n;

        QVector3D Kd = m.Kd;
        if (m.is_texture) {
          QColor color(m.map_Kd_img.pixel(UV[0] * m.map_Kd_img.width(), UV[1] * m.map_Kd_img.height()));
          Kd = QVector3D(color.red(), color.green(), color.blue()) / 255;
        }

        QVector3D L(0,0,0);
        if (m.Ns > 0) {
          L = Li * (m.Ka +                                                  // ambient
              Kd * max(0.0f, QVector3D::dotProduct(n, s)) +                 // diffuse
              m.Ks * powf(max(0.0f, QVector3D::dotProduct(r, v)), m.Ns));   // specular
        } else {
          L = Li * ( m.Ka +                                         // ambient
                Kd * max(0.0f, QVector3D::dotProduct(n, s)) );      // specular      
        }
        red = L[0]; green = L[1]; blue = L[2]; // Phong Shading color
      } else {
        red = 0.15f; green = 0.15f; blue = 0.15f;  // Background color
      }
      // Clamp color values.
      red = min(red, 1.0f); green = min(green, 1.0f); blue = min(blue, 1.0f);
      red = max(red, 0.0f); green = max(green, 0.0f); blue = max(blue, 0.0f);
      QRgb value = qRgb(red*255.0, green*255.0, blue*255.0) ;
      output.setPixel(imgx, imgy, value);
    }
  }
  output.save(pngFile);
  auto t1 = chrono::high_resolution_clock::now();
  cout << "Rendering took " << chrono::duration_cast<chrono::milliseconds>(t1 - t0).count() / 1000. << " seconds." << endl;;

  QString message = QString::number(aabb_cnt) + " ray-AABB intersections and "  + QString::number(tri_cnt)  + " ray-triangle intersections";
  QMessageBox::information(window(), "Intersect counts", message);

  if (useBVH) mesh->clearBVH();
  
  return true;
}
