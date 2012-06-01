#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "ui_VideoPlayer.h"
#include <vector>
#include <QMainWindow>
#include <QAudioOutput>
#include <QTimer>
#include "AVInfo.hpp"
#include "FrameFifo.hpp"

class QGLCanvas;
class LibavWorker;

class VideoPlayer : public QMainWindow, private Ui::VideoPlayer
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

signals:
    void failAvFormat();

public slots:
    void startPlay( AVInfo aAvInfo );
    void fetchAndPlay();

private:
    void setupConnection();
    QAudioFormat getAudioFormat( AVInfo const & aAvInfo ) const;
    double getAudioPlayedSecond() const;
    double getRenewPeriod( double a_fps ) const;

    // the decoded thread
    LibavWorker * mLibavWorker;

    // the widget to play video
    QGLCanvas * videoCanvas;

    // the objects to play audio
    QAudioOutput * mAudioOutput;
    QIODevice * mOutputDevice;
    std::vector<uint8> mAudioStreamBuffer;

    // the infomation of the current playing multimedia
    AVInfo mCurrentAvInfo;

    // timer to fetch decoded frame
    QTimer mTimer;
};

#endif // VIDEOPLAYER_H
