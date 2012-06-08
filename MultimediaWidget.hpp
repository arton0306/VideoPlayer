#ifndef MULTIMEDIAWIDGET_HPP
#define MULTIMEDIAWIDGET_HPP

#include "ui_MultimediaWidget.h"
#include <QWidget>
#include <vector>
#include <QMainWindow>
#include <QAudioOutput>
#include <QTimer>
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

#endif // MULTIMEDIAWIDGET_HPP
