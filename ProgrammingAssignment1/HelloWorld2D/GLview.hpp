#ifndef __GLVIEW_HPP__
#define __GLVIEW_HPP__

#include <QtGui>
#include <QOpenGLWidget>
#include <vector>
#include <iostream>

class GLview : public QOpenGLWidget, protected QOpenGLFunctions  {
    Q_OBJECT
public:
    // this is the class constructor
    GLview(QWidget *parent = 0);

    // this is the class destructor. any memory created in the class should be deleted by this function
    ~GLview();

    // response functions to different key presses.
    void respondToUpKey();
    void respondToDownKey();

protected:
    void initializeGL(); // QGLWidget OpenGL interface
    void paintGL();
    void resizeGL(int width, int height);

    QOpenGLShaderProgram m_shader;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_colorBuffer;
    QOpenGLBuffer m_texBuffer;

    bool g_leftClicked;
    int g_leftClickX;
    int g_leftClickY;
    float g_objScale;
    float seed;
    bool mousePressed;

    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );

    QOpenGLVertexArrayObject vao;
    QOpenGLTexture *texture;

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    /*
    timerEvent() is called every x milliseconds and acts as the main "event loop"
    The timer is initialized in the constructor of this class by startTimer().
    You should call other functions within timerEvent() to update
    information about the animation (xy coordinates, color, etc.).
    */
    void timerEvent(QTimerEvent *event);
};



#endif
