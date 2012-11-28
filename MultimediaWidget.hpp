#ifndef MULTIMEDIAWIDGET_HPP
#define MULTIMEDIAWIDGET_HPP

#include <vector>
#include <QWidget>
#include <QMainWindow>
#include <QAudioOutput>
#include <QTimer>
#include <QTime>
#include <QString>
#include "ui_MultimediaWidget.h"
#include "AVInfo.hpp"
#include "FrameFifo.hpp"

class QGLCanvas;
class LibavWorker;

class MultimediaWidget : public QWidget, private Ui::MultimediaWidget
{
    Q_OBJECT

public:
    explicit MultimediaWidget(QWidget *parent = 0);
    ~MultimediaWidget();

signals:
    void failAvFormat();

public slots:
    void getDecodeReadySignal( AVInfo aAvInfo );
    void getDecodeDoneSignal();

    void renew();
    void play( QString aFileName );
    void seek( int aMSec );
    void stop();

private:
    void setupConnection();
    QAudioFormat getAudioFormat( AVInfo const & aAvInfo ) const;
    double getAudioPlayedSecond() const;
    double getRenewPeriod( double a_fps ) const;

    static int const CHECK_RENEW_MSEC = 5;

    // the decoded thread
    LibavWorker * mLibavWorker;

    // the widget to play video
    QGLCanvas * mVideoCanvas;

    // the objects to play audio
    QAudioOutput * mAudioOutput;
    QIODevice * mOutputDevice;
    std::vector<uint8> mAudioStreamBuffer;

    // the infomation of the current playing multimedia
    AVInfo mCurrentAvInfo;

    // timer to fetch decoded frame
    QTimer mTimer;

    // outside clock, independent to audio and video time
    // The QTime is so suck that it can not be adjusted and start correctly, we use a var to deal with it
    mutable QTime mOutsideTime;
    mutable int mAdjustMs;

    // flag
    bool mIsDecodeDone;
};

#endif // MULTIMEDIAWIDGET_HPP
