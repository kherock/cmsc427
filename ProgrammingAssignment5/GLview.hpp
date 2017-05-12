#ifndef __GLVIEW_HPP__
#define __GLVIEW_HPP__

#include <QtGui>
#include <QOpenGLWidget>

#include "Mesh.hpp"

class GLview : public QOpenGLWidget, protected QOpenGLFunctions  {
  Q_OBJECT
public:
  GLview(QWidget *parent = 0);
  ~GLview() {
    makeCurrent();
    if(mesh != NULL) delete mesh;
    doneCurrent();
  }
  void keyPressGL(QKeyEvent* e);
  bool LoadOBJFile(const QString file, const QString path);
  void toggleLightMotion() { if(mesh == NULL) return;  lightMotionFlag = !lightMotionFlag;  }
  bool Render(QString pngFile, bool useBVH, bool useSAH);
protected:
  void initializeGL(); // QGLWidget OpenGL interface
  void initLightCameraGL();
  void paintGL();
  void resizeGL(int width, int height);

  Mesh *mesh;  // Geometry data.

  QOpenGLShaderProgram texture_shader;

  // Camera parameters --------------
  QVector3D eye, lookCenter, lookUp; 
  QQuaternion camrot;
  float yfov, neardist, fardist;  
  // -------------------------------

  // Light parameters ------
  QVector3D LightDirection, LightIntensity;

  QOpenGLVertexArrayObject vao;
  bool prepareShaderProgram(QOpenGLShaderProgram &shader, 
			    const QString &vertex_file, 
			    const QString &fragment_file);

  bool prepareShaderProgramTex();

  // Used to track mouse position for keyboard shortcuts.
  bool lastPosFlag; // Flag is true if a last position has been captured. Set to false 
                    // prior to mouse driven interaction.
  float lastPosX, lastPosY;   // Mouse state information. 
  bool scaleFlag, translateFlag, rotateFlag;  // Camera movement state information.

  bool lightMotionFlag;

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

  int resize_width, resize_height;
  QElapsedTimer elapsed_time;
  double timeAccumulator, totalTime;
  void updateGLview(float dt);

  void timerEvent(QTimerEvent *event);
};

#endif

