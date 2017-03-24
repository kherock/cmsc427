//#define GL_GLEXT_PROTOTYPES

#include "GLview.hpp"
#include <algorithm>
#include <QTimer>
#include <QtMath>

using namespace std;

GLview::GLview(QWidget *parent)  : QOpenGLWidget(parent)
{
  g_objScale = 1;
  seed = 0.33983690945; // arcsin(1/3)
  texture = NULL;
  mousePressed = false;
  startTimer(15, Qt::PreciseTimer);
}


GLview::~GLview()
{
    makeCurrent();
    if(texture != NULL) {
        delete texture;
    }
    doneCurrent();
}


void GLview::initializeGL()
{
    initializeOpenGLFunctions();
    vao.create();
    if (vao.isCreated()) {
        vao.bind();
    }

    // Set the clear color to black
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    // Prepare a complete shader program...
    if ( !prepareShaderProgram( ":/simple.vsh", ":/simple.fsh" ) ) return;

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !m_shader.bind() ) {
        qWarning() << "Could not bind shader program to context";
        return;
    }

    // define the xy vertices for two triangles which will make up the rectangle
    GLfloat sqVerts[12] = {
        -.5, -.5,
        .5,  .5,
        .5,  -.5,

        -.5, -.5,
        -.5, .5,
        .5,  .5
    };
    // create the vertex buffer and load the data into it
    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vertexBuffer.bind() ) {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }
    m_vertexBuffer.allocate( sqVerts, 6 * 2 * sizeof( float ) );

    m_shader.setAttributeBuffer( "aVertex", GL_FLOAT, 0, 2 );
    m_shader.enableAttributeArray( "aVertex" );


    // define the RGB values (range of 0-1) of each vertex of the two triangles
    GLfloat sqCol[18] =  {
        1, 0, 0,
        0, 1, 1,
        0, 0, 1,

        1, 0, 0,
        0, 1, 0,
        0, 1, 1
    };
    // create the color buffer and load the data into it
    m_colorBuffer.create();
    m_colorBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_colorBuffer.bind() )  {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }
    m_colorBuffer.allocate( sqCol, 6 * 3 * sizeof( float ) );

    m_shader.setAttributeBuffer( "aColor", GL_FLOAT, 0, 3 );
    m_shader.enableAttributeArray( "aColor" );

    // define the xy texture coordinates of each vertex of the two triangles
    GLfloat sqTex[12] = {
        0, 0,
        1,  1,
        1,  0,

        0, 0,
        0, 1,
        1,  1
    };
    m_texBuffer.create();
    m_texBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_texBuffer.bind() )  {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }
    m_texBuffer.allocate( sqTex, 6 * 2 * sizeof( float ) );

    m_shader.setAttributeBuffer( "aTexCoord", GL_FLOAT, 0, 2 );
    m_shader.enableAttributeArray( "aTexCoord" );

    texture = new QOpenGLTexture(QImage(":/reachup.png").mirrored());
    //texture->bind(0);
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Uniforms must be set last.
    m_shader.bind();
    m_shader.setUniformValue("uVertexScale", (float)g_objScale);
    m_shader.setUniformValue("uTexUnit0", (int)0);
}


void GLview::resizeGL( int w, int h )
{
  // Set the viewport to window dimensions
  glViewport( 0, 0, w, qMax( h, 1 ) );
}


void GLview::paintGL()
{
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_shader.setUniformValue("uVertexScale", (float)g_objScale);
    m_shader.setUniformValue("uTexUnit0", (int)0);

    texture->bind(0);
    // Draw stuff
    glDrawArrays( GL_TRIANGLES, 0, 6 );
}


bool GLview::prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath )
{
    // First we load and compile the vertex shader...
    bool result = m_shader.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath );
    if ( !result )
        qWarning() << m_shader.log();

    // ...now the fragment shader...
    result = m_shader.addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath );
    if ( !result )
        qWarning() << m_shader.log();

    // ...and finally we link them to resolve any references.
    result = m_shader.link();
    if ( !result )
        qWarning() << "Could not link shader program:" << m_shader.log();

    return result;
}


void GLview::mousePressEvent(QMouseEvent *event)
{
  g_leftClicked = true;
  g_leftClickX = event->x();
  g_leftClickY = height() - event->y() - 1;
  event->accept();
  mousePressed = true;
  update();
}


void GLview::mouseMoveEvent(QMouseEvent *event)
{
  const int newx = event->x();
  const int newy = height() - event->y() - 1;
  if (g_leftClicked) {
    float deltax = (newx - g_leftClickX) * 0.02;
    g_objScale += deltax;
    g_leftClickX = newx;
    g_leftClickY = newy;
  }
  event->accept();
  update();
}


void GLview::mouseReleaseEvent(QMouseEvent *event)
{
  g_leftClicked = false;
  event->accept();
  update();
  mousePressed = false;
}


void GLview::respondToUpKey()
{
  cout << "GLview::respondToUpKey()" << endl;
}


void GLview::respondToDownKey()
{
  cout << "GLview::respondToDownKey()" << endl;
}


void GLview::timerEvent(QTimerEvent *)
{
  if(mousePressed == false) {
    seed += 0.01;
    g_objScale = 3*qSin(seed);
    update();
  }
}
