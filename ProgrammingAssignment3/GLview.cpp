#include "GLview.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <math.h>
#include <unordered_map>
#include <time.h>

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
GLview::GLview(QWidget *parent)  : QOpenGLWidget(parent)
{
    scaleFlag = translateFlag = rotateFlag = false;
    lastPosFlag = false;
    mesh = NULL;
}

GLview::~GLview()
{
    makeCurrent(); // When deleting the mesh, you need to grab and relase the opengl context.
    if(mesh != NULL) {
        delete mesh;
    }
    doneCurrent();
}

// Load object file. DO NOT MODIFY.
bool GLview::LoadOBJFile(const QString file)
{
    Mesh *newmesh = new Mesh;
    if(!newmesh->load_obj(file)) {
        makeCurrent(); // Make current openGL context.
        delete newmesh;
        doneCurrent(); // Make current openGL context.
        return false;
    }
    if(mesh != NULL) {
        makeCurrent(); // Make current openGL context.
        delete mesh;
        doneCurrent(); // release openGL context.
    }
    mesh = newmesh;
    update_mesh();
    return true;
}

// Set default GL parameters.
void GLview::initializeGL()
{
    initializeOpenGLFunctions();
    vao.create();
    if (vao.isCreated()) {
        vao.bind();
    }

    glClearColor(0.2, 0.2, 0.2, 1.0f );   // Set the clear color to black
    glEnable(GL_DEPTH_TEST);    // Enable depth buffer

    // Prepare a complete shader program...
    if ( !prepareShaderProgram(wire_shader,  ":/wireframe.vsh", ":/wireframe.fsh" ) ) return;

    // Initialize default camera parameters
    yfov = 55;
    neardist = 0.1; fardist = 100;
    eye = QVector3D(-3,3,3);
    lookCenter = QVector3D(0,0,0);
    lookUp = QVector3D(0,0,1);

    QMatrix4x4 view;
    view.lookAt(eye, lookCenter, lookUp);
    Matrix2Quaternion(camrot, view);
}

// Set the viewport to window dimensions. DO NOT MODIFY.
void GLview::resizeGL( int w, int h )
{
    glViewport( 0, 0, w, qMax( h, 1 ) );
}

void GLview::initCameraGL()
{
    QMatrix4x4 model, view, projection;

    // model.rotate(QQuaternion::slerp(q0slerp, q1slerp, slerpt).normalized());

    view.rotate(camrot); view.translate(-eye);
    projection.perspective(yfov, (float)width() / (float)height(), neardist, fardist);

    QMatrix4x4 model_view = view * model;
    QMatrix4x4 MVP = projection * model_view;

    wire_shader.bind();
    wire_shader.setUniformValue("MVP", MVP);
}

void GLview::paintGL()
{
    initCameraGL(); // Update lighting and camara position.

    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if(mesh != NULL) {  wire_shader.bind(); glDrawArrays( GL_TRIANGLES, 0, 3*mesh->faces.size() );  }
}

void GLview::keyPressGL(QKeyEvent* e)
{
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


// DO NOT MODIFY
bool GLview::prepareShaderProgram(QOpenGLShaderProgram &prep_shader, const QString &vertex_file, const QString &fragment_file)
{
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


// DO NOT MODIFY
void GLview::mousePressEvent(QMouseEvent *event)
{
    float px = event->x(), py = event->y();
    if (event->button()==Qt::RightButton) {
        ShowContextMenu(event);
    }
    if (event->button()==Qt::LeftButton) {
        if(mesh == NULL) return;
        float x = 2.0 * (px + 0.5) / float(width())  - 1.0,  y = -(2.0 * (py + 0.5) / float(height()) - 1.0);
        lastPosX = x;
        lastPosY = y;
        lastPosFlag = true;
        event->accept();
    }
}


// show the right-click popup menu. DO NOT MODIFY
void GLview::ShowContextMenu(const QMouseEvent *event)
{
  // You made add additional menu options for the extra credit below,
  // please use the template below to add additional menu options.
    QMenu menu;

    QAction* option0 = new QAction("Example", this);
    connect(option0, SIGNAL(triggered()), this, SLOT(process_example()));
    QAction* option1 = new QAction("Inflate", this);
    connect(option1, SIGNAL(triggered()), this, SLOT(inflate()));
    QAction* option2 = new QAction("Random Noise", this);
    connect(option2, SIGNAL(triggered()), this, SLOT(randomNoise()));
    QAction* option3 = new QAction("Split faces", this);
    connect(option3, SIGNAL(triggered()), this, SLOT(splitFaces()));
    QAction* option4 = new QAction("Center vertices tangentially", this);
    connect(option4, SIGNAL(triggered()), this, SLOT(centerVerticesTangentially()));
    QAction* option5 = new QAction("Split long edges", this);
    connect(option5, SIGNAL(triggered()), this, SLOT(splitLongEdges()));
    QAction* option6 = new QAction("Collapse short edges", this);
    connect(option6, SIGNAL(triggered()), this, SLOT(collapseShortEdges()));
    QAction* option7 = new QAction("Flip edges", this);
    connect(option7, SIGNAL(triggered()), this, SLOT(flipEdges()));
    QAction* option8 = new QAction("Bilateral Smoothing", this);
    connect(option8, SIGNAL(triggered()), this, SLOT(bilateralSmoothing()));
    QAction* option9 = new QAction("Mesh Simplification", this);
    connect(option9, SIGNAL(triggered()), this, SLOT(meshSimplification()));
    QAction* option10 = new QAction("Crop", this);
    connect(option10, SIGNAL(triggered()), this, SLOT(crop()));
    QAction* option11 = new QAction("Smooth", this);
    connect(option11, SIGNAL(triggered()), this, SLOT(smooth()));
    QAction* option12 = new QAction("Sharpen", this);
    connect(option12, SIGNAL(triggered()), this, SLOT(sharpen()));
    QAction* option13 = new QAction("Truncate", this);
    connect(option13, SIGNAL(triggered()), this, SLOT(truncate()));
    QAction* option14 = new QAction("Loop Subdivision", this);
    connect(option14, SIGNAL(triggered()), this, SLOT(loopSubdivision()));

    menu.addAction(option0);
    menu.addAction(option1);
    menu.addAction(option2);
    menu.addAction(option3);
    menu.addAction(option4);
    menu.addAction(option5);
    menu.addAction(option6);
    menu.addAction(option7);    
    menu.addAction(option8);
    menu.addAction(option9);
    menu.addAction(option10);
    menu.addAction(option11);
    menu.addAction(option12);
    menu.addAction(option13);
    menu.addAction(option14);
    menu.exec(mapToGlobal(event->pos()));
}



// DO NOT MODIFY
void GLview::mouseMoveEvent(QMouseEvent *event)
{
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
    update(); // Update display.
}

// To implement your methods, you should add a function to mesh and call as below.
void GLview::process_example()
{
    if(mesh == NULL) return;
    mesh->process_example();
    update_mesh();
}


void GLview::update_mesh()
{
    if(mesh == NULL) return;
    makeCurrent();
    mesh->storeVBO();
    mesh->updateVertices();

    // Update VBOs associated with shaders.
    wire_shader.bind();
    mesh->vertexBuffer.bind();
    wire_shader.setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 3 );
    wire_shader.enableAttributeArray( "VertexPosition" );

    mesh->baryBuffer.bind();
    wire_shader.setAttributeBuffer( "Barycentric", GL_FLOAT, 0, 3 );
    wire_shader.enableAttributeArray( "Barycentric" );
    doneCurrent();

    update();
}


// DO NOT MODIFY
void GLview::toggleRotate()
{
    if(mesh == NULL) return;
    translateFlag = scaleFlag = false;
    rotateFlag = !rotateFlag;
    setMouseTracking(rotateFlag);
    lastPosFlag = false;
}

// DO NOT MODIFY
void GLview::toggleScale()
{
    if(mesh == NULL) return;
    translateFlag = rotateFlag = false;
    scaleFlag = !scaleFlag;
    setMouseTracking(scaleFlag);
    lastPosFlag = false;
}

// DO NOT MODIFY
void GLview::toggleTranslate()
{
    if(mesh == NULL) return;
    rotateFlag = scaleFlag = false;
    translateFlag = !translateFlag;
    setMouseTracking(translateFlag);
    lastPosFlag = false;
}




// Note. After updating/modifying the mesh, you'll need to call update_mesh() below.

void GLview::inflate() {
  // popup dialog box to get user input
  bool ok;
  double factor = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"), tr("Factor:"), 0, -5,
                                          numeric_limits<double>::max(), 2, &ok);
  if (!ok) {
    // warning message to notify user of bad input
    QMessageBox::information(this, tr("Application Name"),
                             tr("Input value not in acceptable range. Please try a different value."));
    return;
  }
  if (mesh == NULL) return;
  for (int i = 0; i < mesh->vertices.size(); i++) {
    mesh->vertices[i].v += mesh->vertices[i].normal * factor * mesh->vertices[i].avgEdgeLen;
  }
  update_mesh();
}

void GLview::randomNoise() {
  // popup dialog box to get user input
  bool ok;
  double factor = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"), tr("Factor:"), 0,
                                          numeric_limits<double>::min(), numeric_limits<double>::max(), 2, &ok);
  if (!ok) {
    // warning message to notify user of bad input
    QMessageBox::information(this, tr("Application Name"),
                             tr("Input value not in acceptable range. Please try a different value."));
    return;
  }
  if (mesh == NULL) return;
  srand(time(NULL));
  for (int i = 0; i < mesh->vertices.size(); i++) {
    QVector3D randv = QVector3D(rand(), rand(), rand()).normalized();
    double len = factor * mesh->vertices[i].avgEdgeLen * (double)rand() / RAND_MAX;
    mesh->vertices[i].v += randv * len;
  }
  update_mesh();
}

void GLview::splitFaces() {
  unordered_map<int, int> edgesSplit;
  int size = mesh->faces.size();
  for (int i = 0; i < size; i++) {
    Mesh_Face &f = mesh->faces[i];
    long midpoints [3];
    for (int j0 = 0; j0 < 3; j0++) {
      int j1 = j0 == 2 ? 0 : j0 + 1;
      // Use Knuth's hash on the indices to keep track of split edges
      int key = (min(f.vert[j0], f.vert[j1]) * 2654435761U) ^ max(f.vert[j0], f.vert[j1]);
      unordered_map<int,int>::const_iterator value = edgesSplit.find(key);
      if (value == edgesSplit.end()) {
        edgesSplit[key] = midpoints[j0] = mesh->split_edge(f.vert[j0], f.vert[j1]);
      } else {
        midpoints[j0] = value->second;
      }
    }
    // Make the new faces
    Vertex &v0 = mesh->vertices[f.vert[0]];
    Vertex &v1 = mesh->vertices[f.vert[1]];
    Vertex &v2 = mesh->vertices[f.vert[2]];
    
    Mesh_Face f0 = Mesh_Face(f.vert[0], midpoints[0], midpoints[2]); //     v0    
    Mesh_Face f1 = Mesh_Face(f.vert[1], midpoints[1], midpoints[0]); //   m0  m2  
    Mesh_Face f2 = Mesh_Face(f.vert[2], midpoints[2], midpoints[1]); // v1  m1  v2
    // Update the old face with the new middle face
    f.vert[0] = midpoints[0];
    f.vert[1] = midpoints[1];
    f.vert[2] = midpoints[2];
    for (int j = 0; j < v0.faces.size(); j++) {
      if (v0.faces[j] == i) {
        v0.faces[j] = mesh->faces.size();
        break;
      }
    }
    mesh->faces.push_back(f0);
    for (int j = 0; j < v1.faces.size(); j++) {
      if (v1.faces[j] == i) {
        v1.faces[j] = mesh->faces.size();
        break;
      }
    }
    mesh->faces.push_back(f1);
    for (int j = 0; j < v2.faces.size(); j++) {
      if (v2.faces[j] == i) {
        v2.faces[j] = mesh->faces.size();
        break;
      }
    }
    mesh->faces.push_back(f2);
  }
  update_mesh();
}

void GLview::starFaces() { cout << "implement starFaces()\n"; }

void GLview::splitLongEdges() { cout << "implement splitLongEdges()\n"; }

void GLview::collapseShortEdges() { cout << "implement collapseShortEdges()\n"; }

void GLview::crop() { cout << "implement crop()\n"; }

void GLview::centerVerticesTangentially() { cout << "implement centerVerticesTangentially()\n"; }

void GLview::sharpen() {
  vector<Vertex> out;
  out.resize(mesh->vertices.size());
  for (int i = 0; i < mesh->vertices.size(); i++) {
    Vertex &v = mesh->vertices[i];
    float sigma = mesh->vertices[i].avgEdgeLen;
    double variance2 = sigma * sigma * 2.0;
    float totalWeight = 0;
    float weight;

    vector<float> weights; // weights of the neighboring vertices
    weights.resize(v.edges.size());

    weight = 1 / sqrt(M_PI * variance2); // weight of the vertex being processed
    totalWeight += weight;
    out[i].v += weight * v.v;

    for (int j = 0; j < v.edges.size(); j++) {
      float distance = (v.v - mesh->vertices[v.edges[j]].v).length();
      weight = exp(-(distance * distance) / variance2) / sqrt(M_PI * variance2);
      totalWeight += weight;
      out[i].v += weight * mesh->vertices[v.edges[j]].v;
    }
    // Normalize the summed coordinates so that the weights sum to 1
    out[i].v /= totalWeight;
    out[i].v += 2 * (mesh->vertices[i].v - out[i].v);
    out[i].edges.swap(mesh->vertices[i].edges);
    out[i].faces.swap(mesh->vertices[i].faces);
  }
  mesh->vertices.clear();
  mesh->vertices.swap(out);
  update_mesh();
}

void GLview::truncate() { cout << "implement truncate()\n"; }

void GLview::bilateralSmoothing() { cout << "implement bilateralSmoothing()\n"; }

void GLview::meshSimplification() { cout << "implement meshSimplification()\n"; }

void GLview::loopSubdivision() { cout << "implement loopSubdivision()\n"; }

void GLview::flipEdges() { cout << "implement flipEdges()\n"; }

void GLview::smooth() {
  vector<Vertex> out;
  out.resize(mesh->vertices.size());
  for (int i = 0; i < mesh->vertices.size(); i++) {
    Vertex &v = mesh->vertices[i];
    float sigma = mesh->vertices[i].avgEdgeLen;
    double variance2 = sigma * sigma * 2.0;
    float totalWeight = 0;
    float weight;

    vector<float> weights; // weights of the neighboring vertices
    weights.resize(v.edges.size());

    weight = 1 / sqrt(M_PI * variance2); // weight of the vertex being processed
    totalWeight += weight;
    out[i].v += weight * v.v;

    for (int j = 0; j < v.edges.size(); j++) {
      float distance = (v.v - mesh->vertices[v.edges[j]].v).length();
      weight = exp(-(distance * distance) / variance2) / sqrt(M_PI * variance2);
      totalWeight += weight;
      out[i].v += weight * mesh->vertices[v.edges[j]].v;
    }
    // Normalize the summed coordinates so that the weights sum to 1
    out[i].v /= totalWeight;
    out[i].edges.swap(mesh->vertices[i].edges);
    out[i].faces.swap(mesh->vertices[i].faces);
  }
  mesh->vertices.clear();
  mesh->vertices.swap(out);
  update_mesh();
}
