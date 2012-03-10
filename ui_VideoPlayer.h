/********************************************************************************
** Form generated from reading UI file 'VideoPlayer.ui'
**
** Created: Sat Mar 10 15:36:01 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIDEOPLAYER_H
#define UI_VIDEOPLAYER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VideoPlayer
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *VideoPlayer)
    {
        if (VideoPlayer->objectName().isEmpty())
            VideoPlayer->setObjectName(QString::fromUtf8("VideoPlayer"));
        VideoPlayer->resize(400, 300);
        menuBar = new QMenuBar(VideoPlayer);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        VideoPlayer->setMenuBar(menuBar);
        mainToolBar = new QToolBar(VideoPlayer);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        VideoPlayer->addToolBar(mainToolBar);
        centralWidget = new QWidget(VideoPlayer);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        VideoPlayer->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(VideoPlayer);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        VideoPlayer->setStatusBar(statusBar);

        retranslateUi(VideoPlayer);

        QMetaObject::connectSlotsByName(VideoPlayer);
    } // setupUi

    void retranslateUi(QMainWindow *VideoPlayer)
    {
        VideoPlayer->setWindowTitle(QApplication::translate("VideoPlayer", "VideoPlayer", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VideoPlayer: public Ui_VideoPlayer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOPLAYER_H
