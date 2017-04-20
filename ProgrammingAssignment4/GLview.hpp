#ifndef __GLVIEW_HPP__
#define __GLVIEW_HPP__

#include <QtGui>
#include <QOpenGLWidget>

#include "Mesh.hpp"

#include <iostream>

class GLview : public QOpenGLWidget, protected QOpenGLFunctions  {
  Q_OBJECT
public:
  GLview(QWidget *parent = 0);
  ~GLview() {
    makeCurrent(); // need opengl context to free buffers, otherwise segfault
    if(mesh != NULL) delete mesh;
    doneCurrent();
  }
  void keyPressGL(QKeyEvent* e);
  bool LoadOBJFile(const QString file, const QString path);
protected:
  void initializeGL(); // QGLWidget OpenGL interface
  void paintGL();
  void resizeGL(int width, int height);

  Mesh *mesh = NULL;  // Geometry data.

  // Camera parameters --------------
  QVector3D eye; // camera position
  QVector3D lookCenter, lookUp;  // Camera look center and lookUp direction fixed at [0,0,1]'
  QQuaternion camrot; // rotation that maps camera to look at -z direction.
  float yfov, neardist, fardist;  // FOV and near and far plane distance
  // -------------------------------

  // Directional Light parameters ------
  QVector3D LightDirection, LightIntensity;
  // -----------------------

  QOpenGLShaderProgram shaders;
  QOpenGLVertexArrayObject vao;
  bool prepareShaderProgram(QOpenGLShaderProgram &shader, 
			    const QString &vertex_file, 
			    const QString &fragment_file);

  // Used to track mouse position for keyboard shortcuts.
  bool lastPosFlag = false; // Flag is true if a last position has been captured. Set to false 
                            // prior to mouse driven interaction.
  float lastPosX, lastPosY;   // Mouse state information. 
  bool scaleFlag = false, translateFlag = false, rotateFlag = false;  // Camera movement state information.

  bool lightMotionFlag = false;

  void mousePressEvent(QMouseEvent *event); 
  void mouseMoveEvent(QMouseEvent *event);
  void toggleRotate() { 
    if(mesh == NULL) return;
    translateFlag = scaleFlag = false;
    rotateFlag = !rotateFlag; setMouseTracking(rotateFlag); lastPosFlag = false; 
  }
  void toggleScale() { 
    if(mesh == NULL) return;
    translateFlag = rotateFlag = false;
    scaleFlag = !scaleFlag; setMouseTracking(scaleFlag); lastPosFlag = false; 
  }
  void toggleTranslate() { 
    if(mesh == NULL) return;
    rotateFlag = scaleFlag = false;
    translateFlag = !translateFlag; setMouseTracking(translateFlag);  lastPosFlag = false; 
  }

  QElapsedTimer elapsed_time;
  double timeAccumulator = 0, totalTime = 0;
  void updateGLview(float dt);				   
  void timerEvent(QTimerEvent *event);

private:
  void ShowContextMenu(const QMouseEvent *event);

private slots:
  void light_motion();
  // implement these operations
  void animate_fov();
  void animate_near();
  void animate_far();
  void animate_camera();
  void cycle_material();
  void animate_material();
  void cycle_group();
  void animate_rotate_wheels();
  void animate_swerve_wheels();  
  
};

#endif

