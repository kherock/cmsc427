#include "GLview.hpp"

#include <iostream>

using namespace std;

GLview::GLview(QWidget *parent)  : QOpenGLWidget(parent)
{
    g_objScale = 1;
    seed = 0.33983690945; //arcsin(1/3)
    mousePressed = false;
    // start a timer. A timer event will occur every 17 milliseconds
    startTimer(17, Qt::PreciseTimer);
}


// empty destructor -- don't write anything for this function
GLview::~GLview(){}


void GLview::initializeGL()
{
    initializeOpenGLFunctions();
    vao.create();
    if (vao.isCreated()) {
        vao.bind(); // need this to initialize GL state.
    }


    // Set the clear color to black
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );


    // Prepare a complete shader program...
    if ( !prepareShaderProgram( ":/simple.vsh", ":/simple.fsh" ) ) return;


    // number of vertices (for a triangle, set to 3)
    nVert = 3;


    // Set up vertex and attribute buffers
    float dist = (float)2 / 3;
    float sqVerts[12] = {
       -dist, -dist, // xy coordinates of the first vertex
        dist,  dist, // xy coordinates of the second vertex
        dist, -dist, // xy coordinates of the third vertex
    };
    vertexBuffer.create();
    vertexBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    vertexBuffer.bind();
    vertexBuffer.allocate( sqVerts, nVert * 2 * sizeof( float ) );


    float sqCol[18] =  {
        1, 0, 0, // RGB value of the first vertex
        0.8, 0.2, 0.9, // RGB value of the second vertex
        0, 0, 1, // RGB value of the third vertex
    };
    colorBuffer.create();
    colorBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    colorBuffer.bind();
    colorBuffer.allocate( sqCol, nVert * 3 * sizeof( float ) );


    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    shader.bind();

    vertexBuffer.bind();
    shader.setAttributeBuffer( "aVertex", GL_FLOAT, 0, 2 );
    shader.enableAttributeArray( "aVertex" );

    colorBuffer.bind();
    shader.setAttributeBuffer( "aColor", GL_FLOAT, 0, 3 );
    shader.enableAttributeArray( "aColor" );
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
    shader.bind();
    shader.setUniformValue("uVertexScale", (float)g_objScale);
    // Draw stuff and run shaders
    glDrawArrays( GL_TRIANGLES, 0, nVert );
}


void GLview::updateGeometry()
{
    makeCurrent();
    float updated_vert[2] = {1,-1};
    vertexBuffer.bind();
    vertexBuffer.write(2*2*sizeof(float), updated_vert, sizeof(float) * 2);
    doneCurrent();
}


bool GLview::prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath )
{
    // First we load and compile the vertex shader...
    bool result = shader.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath );
    if ( !result )
    qWarning() << shader.log();

    // ...now the fragment shader...
    result = shader.addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath );
    if ( !result ) qWarning() << shader.log();

    // ...and finally we link them to resolve any references.
    result = shader.link();
    if ( !result )
    qWarning() << "Could not link shader program:" << shader.log();

    return result;
}


void GLview::mousePressEvent(QMouseEvent *event)
{
    g_leftClicked = true;
    g_leftClickX = event->x();
    g_leftClickY = height() - event->y() - 1;
    event->accept();
    mousePressed = true;
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
    mousePressed = false;
    update();
}


void GLview::timerEvent(QTimerEvent *)
{
    // Note how g_objScale is modified each time timerEvent() gets executed
    if(mousePressed == false) {
        seed += 0.05;
        g_objScale = 0.5 * qSin(seed) + 1;
        update();
    }
}


void GLview::keyPressGL(QKeyEvent* e)
{
    switch ( e->key() ) {
        case Qt::Key_Up:
            updateGeometry();
            break;
        case Qt::Key_2:
            moreTriangles();
            break;
    }
}


void GLview::moreTriangles()
{
    makeCurrent();
    cout << "more triangles" << endl;

    nVert = NUM_TRIANGLES * 3;

    int sqVertsLen = NUM_TRIANGLES * 3 * 2;
    float sqVerts[NUM_TRIANGLES*3*2];
    for (int i = 0; i < sqVertsLen; i += 6) {
        sqVerts[i] = sqVerts[i + 1] = 0;
    }
    float theta = 0;
    sqVerts[2] = sqVerts[sqVertsLen - 2] = cos(theta) * 2 / 3;
    sqVerts[3] = sqVerts[sqVertsLen - 1] = sin(theta) * 2 / 3;
    for (int i = 6; i < sqVertsLen; i += 6) {
        theta = M_PI * i / nVert;
        sqVerts[i + 2] = sqVerts[i - 2] = cos(theta) * 2 / 3;
        sqVerts[i + 3] = sqVerts[i - 1] = sin(theta) * 2 / 3;
    }
    int sqColLen = NUM_TRIANGLES * 3 * 3;
    float sqCol[NUM_TRIANGLES*3*3] = {0};
    for (int i = 0; i < sqColLen; i += 9) {
        // Set points at the center to gray
        sqCol[i] = sqCol[i + 1] = sqCol[i + 2] = 0.1;
    }
    float brightness = 0;
    for (int i = 0; i < sqColLen / 2; i += 9) {
        float newBrightness = 1 - abs((float)(i * 2 + 18) / sqColLen - 0.5) * 2;
        int blueOffset = nVert - nVert % 9;
        // Green needs to loop around
        int greenOffset = 2 * nVert - 2 * nVert % 9;
        if (i + greenOffset >= sqColLen) greenOffset -= sqColLen;
        sqCol[i + 3] = sqCol[i + greenOffset + 4] = sqCol[i + blueOffset + 5] = brightness;
        sqCol[i + 6] = sqCol[i + greenOffset + 7] = sqCol[i + blueOffset + 8] = newBrightness;
        brightness = newBrightness;
    }

    vertexBuffer.bind();
    vertexBuffer.destroy();

    colorBuffer.bind();
    colorBuffer.destroy();

    // create an openGL buffer object, bind the buffer to the current OpenGL context, and allocate space to the buffer
    vertexBuffer.create();
    vertexBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    vertexBuffer.bind();
    vertexBuffer.allocate( sqVerts, nVert * 2 * sizeof( float ) );

    colorBuffer.create();
    colorBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    colorBuffer.bind();
    colorBuffer.allocate( sqCol, nVert * 3 * sizeof( float ) );

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    shader.bind();

    vertexBuffer.bind();
    shader.setAttributeBuffer( "aVertex", GL_FLOAT, 0, 2 );
    shader.enableAttributeArray( "aVertex" );

    colorBuffer.bind();
    shader.setAttributeBuffer( "aColor", GL_FLOAT, 0, 3 );
    shader.enableAttributeArray( "aColor" );

    doneCurrent();
}
