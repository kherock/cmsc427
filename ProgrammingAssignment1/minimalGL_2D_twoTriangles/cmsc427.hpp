#ifndef __CMSC427_HPP__
#define __CMSC427_HPP__

#include <QtGui>

#include "ui_cmsc427.h"

class CMSC427Win : public QMainWindow, private Ui::CMSC427Win {
    Q_OBJECT
public:
    CMSC427Win(QMainWindow *parent = 0);
    ~CMSC427Win();
protected:
    void keyPressEvent( QKeyEvent* e );
    private slots:
    // Menu triggers
    void on_action_Quit_triggered();
};

#endif
