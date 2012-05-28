#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "ui_VideoPlayer.h"
#include <QMainWindow>
#include <QAudioOutput>
#include "AVInfo.hpp"

class QGLCanvas;

class VideoPlayer : public QMainWindow, private Ui::VideoPlayer
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

public slots:
    void startPlay( AVInfo aAvInfo );

private:
    void setupConnection();

    // the widget to play video
    QGLCanvas * videoCanvas;

    // the objects to play audio
    QAudioOutput * mAudio;
    QIODevice * mAudioOutputDevice;

    // the infomation of the current playing multimedia
    AVInfo mCurrentAvInfo;
};

#endif // VIDEOPLAYER_H
