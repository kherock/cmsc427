#include "cmsc427.hpp"

CMSC427Win::CMSC427Win(QMainWindow *parent) : QMainWindow(parent) {
    setupUi(this);
}


// empty destructor -- don't write anything for this function
CMSC427Win::~CMSC427Win() {  }


void CMSC427Win::keyPressEvent( QKeyEvent* e )
{
    glwidget->keyPressGL(e);
}


void CMSC427Win::on_action_Quit_triggered()
{
    QApplication::quit();  // Quit.
}


// Application start point.
int main(int argc, char *argv[]) {
    // Launch main application.
    QApplication app(argc, argv);

    // Set OpenGL version and parameters
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
    return app.exec();

}
