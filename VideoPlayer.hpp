#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "ui_VideoPlayer.h"
#include <QMainWindow>

class MultimediaWidget;

class VideoPlayer : public QMainWindow, private Ui::VideoPlayer
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

signals:

public slots:
    void openFile();
    void stop();
    void seek();
    void pitchUp();
    void pitchDown();
    void leftVolUp();
    void leftVolDown();
    void rightVolUp();
    void rightVolDown();

private:
    void setupConnection();
    void setAudioEffect();

    MultimediaWidget * mMultimediaWidget;

    // audio effect
    struct AudioEffect {
        AudioEffect( double key = 0.0, double lc = 1.0, double rc = 1.0 ) {
            mSemiTonesDelta = key;
            mLeftChanVol = lc;
            mRightChanVol = rc;
        }
        double mSemiTonesDelta;
        double mLeftChanVol;  // if the audio is mono, mLeftChanVol == mRightChanVol
        double mRightChanVol;
    } mAudioEffect;
};

#endif // VIDEOPLAYER_H
