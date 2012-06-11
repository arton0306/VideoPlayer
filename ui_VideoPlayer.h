/********************************************************************************
** Form generated from reading UI file 'VideoPlayer.ui'
**
** Created: Mon Jun 11 22:30:53 2012
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
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VideoPlayer
{
public:
    QAction *actionSelectFile;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QGridLayout *mVideoLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *mFileSelect;
    QPushButton *mPlayButton;
    QPushButton *mStop;
    QSlider *horizontalSlider;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton;
    QPushButton *pushButton_4;
    QPushButton *pushButton_8;
    QPushButton *pushButton_9;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_5;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *VideoPlayer)
    {
        if (VideoPlayer->objectName().isEmpty())
            VideoPlayer->setObjectName(QString::fromUtf8("VideoPlayer"));
        VideoPlayer->resize(779, 527);
        actionSelectFile = new QAction(VideoPlayer);
        actionSelectFile->setObjectName(QString::fromUtf8("actionSelectFile"));
        centralWidget = new QWidget(VideoPlayer);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        mVideoLayout = new QGridLayout();
        mVideoLayout->setSpacing(6);
        mVideoLayout->setObjectName(QString::fromUtf8("mVideoLayout"));

        verticalLayout->addLayout(mVideoLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        mFileSelect = new QPushButton(centralWidget);
        mFileSelect->setObjectName(QString::fromUtf8("mFileSelect"));

        horizontalLayout->addWidget(mFileSelect);

        mPlayButton = new QPushButton(centralWidget);
        mPlayButton->setObjectName(QString::fromUtf8("mPlayButton"));

        horizontalLayout->addWidget(mPlayButton);

        mStop = new QPushButton(centralWidget);
        mStop->setObjectName(QString::fromUtf8("mStop"));

        horizontalLayout->addWidget(mStop);

        horizontalSlider = new QSlider(centralWidget);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(horizontalSlider);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_2->addWidget(pushButton);

        pushButton_4 = new QPushButton(centralWidget);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));

        horizontalLayout_2->addWidget(pushButton_4);

        pushButton_8 = new QPushButton(centralWidget);
        pushButton_8->setObjectName(QString::fromUtf8("pushButton_8"));

        horizontalLayout_2->addWidget(pushButton_8);

        pushButton_9 = new QPushButton(centralWidget);
        pushButton_9->setObjectName(QString::fromUtf8("pushButton_9"));

        horizontalLayout_2->addWidget(pushButton_9);

        pushButton_6 = new QPushButton(centralWidget);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));

        horizontalLayout_2->addWidget(pushButton_6);

        pushButton_7 = new QPushButton(centralWidget);
        pushButton_7->setObjectName(QString::fromUtf8("pushButton_7"));

        horizontalLayout_2->addWidget(pushButton_7);

        pushButton_5 = new QPushButton(centralWidget);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));

        horizontalLayout_2->addWidget(pushButton_5);


        verticalLayout->addLayout(horizontalLayout_2);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        VideoPlayer->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(VideoPlayer);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 779, 22));
        VideoPlayer->setMenuBar(menuBar);
        mainToolBar = new QToolBar(VideoPlayer);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        VideoPlayer->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(VideoPlayer);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        VideoPlayer->setStatusBar(statusBar);

        retranslateUi(VideoPlayer);

        QMetaObject::connectSlotsByName(VideoPlayer);
    } // setupUi

    void retranslateUi(QMainWindow *VideoPlayer)
    {
        VideoPlayer->setWindowTitle(QApplication::translate("VideoPlayer", "VideoPlayer", 0, QApplication::UnicodeUTF8));
        actionSelectFile->setText(QApplication::translate("VideoPlayer", "SelectFile", 0, QApplication::UnicodeUTF8));
        mFileSelect->setText(QApplication::translate("VideoPlayer", "File", 0, QApplication::UnicodeUTF8));
        mPlayButton->setText(QApplication::translate("VideoPlayer", "Play", 0, QApplication::UnicodeUTF8));
        mStop->setText(QApplication::translate("VideoPlayer", "Stop", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("VideoPlayer", "Pitch +", 0, QApplication::UnicodeUTF8));
        pushButton_4->setText(QApplication::translate("VideoPlayer", "Pitch -", 0, QApplication::UnicodeUTF8));
        pushButton_8->setText(QApplication::translate("VideoPlayer", "Vol +", 0, QApplication::UnicodeUTF8));
        pushButton_9->setText(QApplication::translate("VideoPlayer", "Vol -", 0, QApplication::UnicodeUTF8));
        pushButton_6->setText(QApplication::translate("VideoPlayer", "Left Chnl", 0, QApplication::UnicodeUTF8));
        pushButton_7->setText(QApplication::translate("VideoPlayer", "Right Chnl", 0, QApplication::UnicodeUTF8));
        pushButton_5->setText(QApplication::translate("VideoPlayer", "Stereo", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VideoPlayer: public Ui_VideoPlayer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOPLAYER_H
