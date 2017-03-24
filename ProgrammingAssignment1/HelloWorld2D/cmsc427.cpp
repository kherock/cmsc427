#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <math.h>
#include "cmsc427.hpp"

using namespace std;

CMSC427Win::CMSC427Win(QMainWindow *parent) : QMainWindow(parent)
{
    setupUi(this);
}

// for now, don't write any code for the destructor
CMSC427Win::~CMSC427Win() {}


void CMSC427Win::keyPressEvent(QKeyEvent* e)
{
    switch ( e->key() ) {
        case Qt::Key_Up:
            glwidget->respondToUpKey();
            break;
        case Qt::Key_Down:
            glwidget->respondToDownKey();
            break;
    }
}


void CMSC427Win::on_action_Quit_triggered()
{
    QApplication::quit();
}


// Application start point.
int main(int argc, char *argv[])
{
    // Launch main application.
    QApplication app(argc, argv);

    // Set OpenGL version
    QSurfaceFormat format;
    format.setSamples(8);
    format.setDepthBufferSize(32);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    // Show main window
    CMSC427Win main_win;
    main_win.show();

    // enter the main event loop by calling exec()
    // when exit() is called, exec() returns the value that was set by exit() (which is 0 if exit() is called via quit()).
    return app.exec();
}
