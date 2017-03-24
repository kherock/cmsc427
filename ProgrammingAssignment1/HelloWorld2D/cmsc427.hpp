#ifndef __EDGEGUI_HPP__
#define __EDGEGUI_HPP__

#include <QtGui>
#include <QtOpenGL>
#include "ui_cmsc427.h"

class CMSC427Win : public QMainWindow, private Ui::CMSC427Win {
    Q_OBJECT
public:
    // this is the class constructor
    CMSC427Win(QMainWindow *parent = 0);

    // this is the class destructor. any memory created in the class should be deleted by this function
    ~CMSC427Win();

protected:
    // this function will capture all key press events.
    // you can modify it to call other functions based on what key (i.e. left arrow, right arrow) was pressed
    void keyPressEvent( QKeyEvent* e );

private slots:
    // send the quit() signal to the main application
    void on_action_Quit_triggered();
};

#endif
