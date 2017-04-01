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

CMSC427Win::~CMSC427Win() {  }

void CMSC427Win::keyPressEvent( QKeyEvent* e )
{
    glwidget->keyPressGL(e);
}

void CMSC427Win::open_OBJ_file()
{
    QString objFile = QFileDialog::getOpenFileName(this, "Open File", "", "3D Object (*.obj)");
    if(objFile == "") return;
    QFileInfo objFileInfo(objFile);
    if(!glwidget->LoadOBJFile(objFile)) {
        QMessageBox::warning(this, "Load OBJ File", "Error Loading OBJ File");
    }
}

void CMSC427Win::on_action_Quit_triggered()
{
    QCoreApplication::instance()->quit();
}

void CMSC427Win::on_action_Open_triggered() { open_OBJ_file();  }

void CMSC427Win::on_pushOpenOBJFile_clicked(bool) { open_OBJ_file(); }

// Application start point.
int main(int argc, char *argv[])
{
    // Launch main application.
    QApplication app(argc, argv);

    // Set OpenGL version
    QSurfaceFormat format;
    format.setDepthBufferSize(32);
    format.setSamples(8);
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
