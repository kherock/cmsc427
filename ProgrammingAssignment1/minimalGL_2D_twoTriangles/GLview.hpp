#ifndef __GLVIEW_HPP__
#define __GLVIEW_HPP__
#define NUM_TRIANGLES 6000

#include <QtGui>
#include <QOpenGLWidget>

class GLview : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    GLview(QWidget *parent = 0);
    ~GLview();

    void updateGeometry();
    void keyPressGL(QKeyEvent* e);

protected:
    void initializeGL(); // QGLWidget OpenGL interface
    void paintGL();
    void resizeGL(int width, int height);

    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgram shader;
    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer colorBuffer;

    bool g_leftClicked;
    int g_leftClickX;
    int g_leftClickY;
    float g_objScale;
    float seed;
    bool mousePressed;
    int nVert;

    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void moreTriangles();

    /*
    timerEvent() is called every x milliseconds and acts as the main "event loop"
    The timer is initialized in the constructor of this class by startTimer().
    You should call other functions within timerEvent() to update
    information about the animation (xy coordinates, color, etc.).
    */
    void timerEvent(QTimerEvent *event);
};

#endif
