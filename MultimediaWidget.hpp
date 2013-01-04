#ifndef MULTIMEDIAWIDGET_HPP
#define MULTIMEDIAWIDGET_HPP

#include <vector>
#include <QWidget>
#include <QMainWindow>
#include <QAudioOutput>
#include <QTimer>
#include <QTime>
#include <QString>
#include "LibavWorker.hpp"
#include "ui_MultimediaWidget.h"
#include "AVInfo.hpp"
#include "FrameFifo.hpp"
#include "VolumeTuner.hpp"

class QGLCanvas;

class MultimediaWidget : public QWidget, private Ui::MultimediaWidget
{
    Q_OBJECT

public:
    explicit MultimediaWidget(QWidget *parent = 0);
    ~MultimediaWidget();

signals:
    void failAvFormat();

public slots:
    void getReadyToDecodeSignal( AVInfo aAvInfo );
    void getDecodeDoneSignal();
    void getSeekStateSignal(bool aIsSuccess);
    void getInitAVFrameReadySignal(double aFirstAudioFrameMsec);

    void updateAV();
    void play( QString aFileName );
    void seek( int aMSec );
    void stop();
    void dumpAVStream();
    void setAudioEffect( int pitchDelta, double leftVol, double rightVol, bool speechMode ) const;

private:
    void setupConnection();
    QAudioFormat getAudioFormat( AVInfo const & aAvInfo ) const;
    void fetchAllAvailableAudioAndPush();
    double getAudioPlayedSecond() const;
    double getRenewPeriod() const;
    int pushAudioStream( std::vector<uint8> const & stream ) const;

    // the decoded thread
    LibavWorker * mLibavWorker;

    // the widget to play video
    QGLCanvas * mVideoCanvas;

    // the interface to play audio
    VolumeTuner * mVolumnTuner;
    AudioPlayer * mAudioPlayer;
    std::vector<uint8> mAudioStreamBuffer;

    // the infomation of the current playing multimedia
    AVInfo mCurrentAvInfo;

    // timer to fetch decoded frame
    QTimer mTimer;

    double mAudioSeekTimeMSec;

    // flag
    bool mIsDecodeDone;
};

#endif // MULTIMEDIAWIDGET_HPP
