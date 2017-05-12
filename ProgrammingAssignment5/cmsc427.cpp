#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include <math.h>

#include "cmsc427.hpp"

using namespace std;

CMSC427Win::CMSC427Win(QMainWindow *parent) : QMainWindow(parent) { 
  setupUi(this); 
}


void CMSC427Win::open_OBJ_file() {
  QString objFile = QFileDialog::getOpenFileName(this, "Open File", "", "3D Object (*.obj)");
  if(objFile == "") return;
  QFileInfo objFileInfo(objFile);
  QString objFilePath = objFileInfo.dir().path();
  if(!glwidget->LoadOBJFile(objFile, objFilePath)) {
    QMessageBox::warning(this, "Load OBJ File", "Error Loading OBJ File");
  }
}
void CMSC427Win::on_action_Open_triggered() { open_OBJ_file();  }

void CMSC427Win::on_pushOpenOBJFile_clicked(bool) { open_OBJ_file(); }



void CMSC427Win::on_pushRender_clicked(bool) {
  QString pngFile = QFileDialog::getSaveFileName(this, "Save File", "", "PNG (*.png)");
  if(pngFile == "") return;

  int BVHmode = comboBVH->currentIndex();
  bool useBVH = false, useSAH = false;
  if(BVHmode == 1 || BVHmode == 2) useBVH = true;
  if(BVHmode == 2) useSAH = true;

  if(!glwidget->Render(pngFile, useBVH, useSAH)) {
    QMessageBox::warning(this, "Error", "Error occured during rendering");
  }
}



// Application start point.
int main(int argc, char *argv[]) {
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
  CMSC427Win main_win; main_win.show();
  return app.exec();
}
