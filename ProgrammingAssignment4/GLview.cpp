#include "GLview.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

float copysign0(float x, float y) { return (y == 0.0f) ? 0 : copysign(x,y); }

void Matrix2Quaternion(QQuaternion &Q, QMatrix4x4 &M) {
  Q.setScalar(sqrt( max( 0.0f, 1 + M(0,0)  + M(1,1) + M(2,2) ) ) / 2);
  Q.setX(sqrt( max( 0.0f, 1 + M(0,0) - M(1,1) - M(2,2) ) ) / 2);
  Q.setY(sqrt( max( 0.0f, 1 - M(0,0) + M(1,1) - M(2,2) ) ) / 2);
  Q.setZ(sqrt( max( 0.0f, 1 - M(0,0) - M(1,1) + M(2,2) ) ) / 2);
  Q.setX(copysign0( Q.x(), M(2,1) - M(1,2) ));
  Q.setY(copysign0( Q.y(), M(0,2) - M(2,0) ));
  Q.setZ(copysign0( Q.z(), M(1,0) - M(0,1) )) ;
}


// GLView constructor. DO NOT MODIFY.
GLview::GLview(QWidget *parent)  : QOpenGLWidget(parent) {
  startTimer(20, Qt::PreciseTimer);
  elapsed_time.start(); elapsed_time.invalidate();  
}  

// Load object file. DO NOT MODIFY.
bool GLview::LoadOBJFile(const QString file, const QString path) {
  makeCurrent();  // Need to grab OpenGL context
  Mesh *newmesh = new Mesh;
  if(!newmesh->load_obj(file, path)) {
    delete newmesh;
    return false;
  }
  if(mesh != NULL) {  delete mesh; }
  mesh = newmesh;
  mesh->storeVBO_groups();

  doneCurrent(); // Release OpenGL context.
  return true;
}

// Set default GL parameters. 
void GLview::initializeGL() {
  initializeOpenGLFunctions();
  vao.create(); if (vao.isCreated()) vao.bind();

  glClearColor( 0.15, 0.15, 0.15, 1.0f );   // Set the clear color to black
  glEnable(GL_DEPTH_TEST);    // Enable depth buffer

  // Prepare a complete shader program...
  if ( !prepareShaderProgram(shaders,  ":/texture.vsh", ":/texture.fsh" ) ) return;

  // Enable buffer names in shader for position, normal and texture coordinates.
  shaders.bind();
  shaders.enableAttributeArray( "VertexPosition" );
  shaders.enableAttributeArray( "VertexNormal" );      
  shaders.enableAttributeArray( "VertexTexCoord" );
  
  // Set default lighting parameters. 
  LightDirection = QVector3D(1,1,0.75);
  LightIntensity = QVector3D(1,1,1);

  // Initialize default camera parameters
  yfov = 55;
  neardist = 1; fardist = 1000;
  eye = QVector3D(-3,3,3); lookCenter = QVector3D(0,0,0); lookUp = QVector3D(0,0,1);
  QMatrix4x4 view; view.lookAt(eye, lookCenter, lookUp);
  Matrix2Quaternion(camrot, view);
}

// Set the viewport to window dimensions. DO NOT MODIFY.
void GLview::resizeGL( int w, int h ) {  glViewport( 0, 0, w, qMax( h, 1 ) ); }

void GLview::paintGL() {
  if(mesh == NULL) return; // Nothing to draw.

  // Clear the frame buffer with the current clearing color and clear depth buffer.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  shaders.bind();
  for(long group_idx = 0; group_idx < (long)mesh->groups.size(); group_idx++) {
    vector<Mesh_Material> &materials = mesh->groups[group_idx].materials;
    for(long mtl_idx = 0; mtl_idx < (long)materials.size(); mtl_idx++) {
      if(materials[mtl_idx].n_triangles == 0) continue;
      QMatrix4x4 model;
      model.translate(mesh->model_translate);
      model.rotate(mesh->model_rotation);
      model.scale(mesh->model_sx, mesh->model_sy, mesh->model_sz);      
      QMatrix4x4 view, projection;

      view.rotate(camrot); view.translate(-eye);
      projection.perspective(yfov, (float)width() / (float)height(), neardist, fardist);
    
      QMatrix4x4 model_view = view * model;
      QMatrix3x3 normal_matrix = model_view.normalMatrix();
      QMatrix4x4 MVP = projection * model_view;

      shaders.setUniformValue("ModelViewMatrix", model_view);
      shaders.setUniformValue("NormalMatrix", normal_matrix);
      shaders.setUniformValue("MVP", MVP);
      shaders.setUniformValue("LightIntensity", LightIntensity);
      // NOTE: Must multiply light position by view matrix!!!!!
      shaders.setUniformValue("LightDirection", view * QVector4D(LightDirection, 0));

      // Bind geometry buffers for current group and material. 
      materials[mtl_idx].vertexBuffer->bind();
      shaders.setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 3 );
      materials[mtl_idx].normalBuffer->bind();
      shaders.setAttributeBuffer( "VertexNormal", GL_FLOAT, 0, 3 );
      materials[mtl_idx].texCoordBuffer->bind();
      shaders.setAttributeBuffer( "VertexTexCoord", GL_FLOAT, 0, 2 );

      // Update shading parameters for current material.
      shaders.setUniformValue("Kd", materials[mtl_idx].Kd);
      shaders.setUniformValue("Ks", materials[mtl_idx].Ks);
      shaders.setUniformValue("Ka", materials[mtl_idx].Ka);
      shaders.setUniformValue("Shininess", materials[mtl_idx].Ns);
      shaders.setUniformValue("useTexture", false);
      
      // If a texture material, so use for drawing.
      if(materials[mtl_idx].map_Kd != NULL) {
	shaders.setUniformValue("useTexture", true);
	// Bind texture to texture unit 0.
	materials[mtl_idx].map_Kd->bind(0); 
	shaders.setUniformValue("Tex1", (int)0); // Update shader uniform.
      }
      // Starting at index 0, draw 3 * n_triangle verticies from current geometry buffers.
      glDrawArrays( GL_TRIANGLES, 0, 3 * materials[mtl_idx].n_triangles );

    }
  }
  
}

void GLview::keyPressGL(QKeyEvent* e) {
  switch ( e->key() ) {
  case Qt::Key_Escape: QCoreApplication::instance()->quit(); break;
    // Set rotate, scale, translate modes (for track pad mainly).
  case Qt::Key_R: toggleRotate();    break;
  case Qt::Key_S: toggleScale();     break;
  case Qt::Key_T: toggleTranslate();     break;
  default: QOpenGLWidget::keyPressEvent(e);  break;
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
  if (event->button()==Qt::RightButton) {
    ShowContextMenu(event);
  }
  if (event->button()==Qt::LeftButton) {
    if(mesh == NULL) return;
    float px = event->x(), py = event->y();
    // Translate to OpenGL coordinates (and middle of pixel).
    float x = 2.0 * (px + 0.5) / float(width())  - 1.0;
    float y = -(2.0 * (py + 0.5) / float(height()) - 1.0); 
    lastPosX = x; lastPosY = y;
    lastPosFlag = true; 
    event->accept();
  }
}

// show the right-click popup menu. DO NOT MODIFY
void GLview::ShowContextMenu(const QMouseEvent *event) {
  // You made add additional menu options for the extra credit below,
  // please use the template below to add additional menu options.
  QMenu menu;

  QAction* option0 = new QAction("Light Direction Motion", this);
  connect(option0, SIGNAL(triggered()), this, SLOT(light_motion()));

  QAction* option1 = new QAction("Animate FOV", this);
  connect(option1, SIGNAL(triggered()), this, SLOT(animate_fov()));

  QAction* option2 = new QAction("Animate Near Plane", this);
  connect(option2, SIGNAL(triggered()), this, SLOT(animate_near()));

  QAction* option3 = new QAction("Animate Far Plane", this);
  connect(option3, SIGNAL(triggered()), this, SLOT(animate_far()));

  QAction* option4 = new QAction("Animate Camera", this);
  connect(option4, SIGNAL(triggered()), this, SLOT(animate_camera()));

  QAction* option5 = new QAction("Cycle Material", this);
  connect(option5, SIGNAL(triggered()), this, SLOT(cycle_material()));

  QAction* option6 = new QAction("Animate Material", this);
  connect(option6, SIGNAL(triggered()), this, SLOT(animate_material()));

  QAction* option7 = new QAction("Cycle Group", this);
  connect(option7, SIGNAL(triggered()), this, SLOT(cycle_group()));

  QAction* option8 = new QAction("Animate Rotate Wheels", this);
  connect(option8, SIGNAL(triggered()), this, SLOT(animate_rotate_wheels()));
  
  QAction* option9 = new QAction("Animate Swerve Wheels", this);
  connect(option9, SIGNAL(triggered()), this, SLOT(animate_swerve_wheels()));
  
  menu.addAction(option0); menu.addAction(option1);
  menu.addAction(option2); menu.addAction(option3);
  menu.addAction(option4); menu.addAction(option5);
  menu.addAction(option6); menu.addAction(option7);
  menu.addAction(option8); menu.addAction(option9);  
  menu.exec(mapToGlobal(event->pos()));
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

    // Get rotation from -z to camera. Rotate the camera in the wold.
    QQuaternion revQ = camrot.conjugate();
    QQuaternion newrot = QQuaternion::fromAxisAndAngle(lookUp, theta_x);
    revQ = newrot * revQ;

    QVector3D side = revQ.rotatedVector(QVector3D(1,0,0));
    QQuaternion newrot2 = QQuaternion::fromAxisAndAngle(side, theta_y);
    revQ = newrot2 * revQ;
    revQ.normalize();

    // Go back to camera frame.
    camrot = revQ.conjugate().normalized();    

    // Update camera position.
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
    // Get camera side and up vectors.
    QQuaternion revQ = camrot.conjugate().normalized();
    QVector3D side = revQ.rotatedVector(QVector3D(1,0,0));
    QVector3D upVector = revQ.rotatedVector(QVector3D(0,1,0));

    // Move camera and look center in direction of the side and up vectors of camera.
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


// Update according to timer. Keep time step fixed. DO NOT MODIFY.
void GLview::timerEvent(QTimerEvent *) {
  if(!elapsed_time.isValid()) {   elapsed_time.restart(); return;  } // Skip first update.
  qint64 nanoSec = elapsed_time.nsecsElapsed();
  elapsed_time.restart();  

  double dt = 0.01; // dt is the animation time step
  double frameTime = double(nanoSec) * 1e-9;

  timeAccumulator += frameTime;
  while ( timeAccumulator >= dt ) {
    // Keep the animation time step fixed, catching up if
    // drawing takes too long. 
    updateGLview(dt);  totalTime += dt;  timeAccumulator -= dt;
  }

  update();  
}

void GLview::light_motion() {
  if(mesh == NULL) return;  lightMotionFlag = !lightMotionFlag; 
}


void GLview::animate_fov() {
  cout << "implement animate_fov()" << endl;
}

void GLview::animate_near() {
  cout << "implement animate_near()" << endl;
}

void GLview::animate_far() {
  cout << "implement animate_far()" << endl;
}

void GLview::animate_camera() {
  cout << "implement animate_camera()" << endl;
}

void GLview::cycle_material() {
  cout << "implement cycle_material()" << endl;

  QString material_name_text = "Material name is here.";
  QMessageBox::information(this, "Material Name", material_name_text);
}

void GLview::animate_material() {
  cout << "implement animate_material()" << endl;
}

void GLview::cycle_group() {
  cout << "implement cycle_group()" << endl;
  QString group_name_text = "Cycle group name here.";
  QMessageBox::information(this, "Group Name", group_name_text);
  
}

void GLview::animate_rotate_wheels() {
  cout << "implement animate_rotate_wheels()" << endl;
  
}

void GLview::animate_swerve_wheels() {
  cout << "implement animate_swerve_wheels()" << endl;
}
