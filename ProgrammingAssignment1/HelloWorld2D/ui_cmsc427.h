/********************************************************************************
** Form generated from reading UI file 'cmsc427.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CMSC427_H
#define UI_CMSC427_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "GLview.hpp"

QT_BEGIN_NAMESPACE

class Ui_CMSC427Win
{
public:
    QAction *action_Quit;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    GLview *glwidget;
    QMenuBar *menubar;
    QMenu *menu_File;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *CMSC427Win)
    {
        if (CMSC427Win->objectName().isEmpty())
            CMSC427Win->setObjectName(QStringLiteral("CMSC427Win"));
        CMSC427Win->resize(800, 600);
        action_Quit = new QAction(CMSC427Win);
        action_Quit->setObjectName(QStringLiteral("action_Quit"));
        centralwidget = new QWidget(CMSC427Win);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        glwidget = new GLview(centralwidget);
        glwidget->setObjectName(QStringLiteral("glwidget"));

        horizontalLayout->addWidget(glwidget);

        CMSC427Win->setCentralWidget(centralwidget);
        menubar = new QMenuBar(CMSC427Win);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        menu_File = new QMenu(menubar);
        menu_File->setObjectName(QStringLiteral("menu_File"));
        CMSC427Win->setMenuBar(menubar);
        statusbar = new QStatusBar(CMSC427Win);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        CMSC427Win->setStatusBar(statusbar);

        menubar->addAction(menu_File->menuAction());
        menu_File->addAction(action_Quit);

        retranslateUi(CMSC427Win);

        QMetaObject::connectSlotsByName(CMSC427Win);
    } // setupUi

    void retranslateUi(QMainWindow *CMSC427Win)
    {
        CMSC427Win->setWindowTitle(QApplication::translate("CMSC427Win", "CMSC427", Q_NULLPTR));
        action_Quit->setText(QApplication::translate("CMSC427Win", "&Quit", Q_NULLPTR));
        menu_File->setTitle(QApplication::translate("CMSC427Win", "&File", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CMSC427Win: public Ui_CMSC427Win {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CMSC427_H
