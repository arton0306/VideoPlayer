#include <cassert>
#include <QTimer>
#include <QFileDialog>
#include "debug.hpp"
#include "VideoPlayer.hpp"
#include "MultimediaWidget.hpp"

using namespace std;

VideoPlayer::VideoPlayer(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    setupConnection();

    mMultimediaWidget = new MultimediaWidget( this );
    mVideoLayout->addWidget( mMultimediaWidget );
}

void VideoPlayer::setupConnection()
{
    connect( mFileSelectBtn, SIGNAL(pressed()), this, SLOT(openFile()) );
    connect( mStopBtn, SIGNAL(pressed()), this, SLOT(stop()) );
    connect( mPlayBtn, SIGNAL(pressed()), this, SLOT(seek()) );
    connect( mPitchUpBtn, SIGNAL(pressed()), this, SLOT(pitchUp()) );
    connect( mPitchDownBtn, SIGNAL(pressed()), this, SLOT(pitchDown()) );
    connect( mLeftVolUpBtn, SIGNAL(pressed()), this, SLOT(leftVolUp()) );
    connect( mLeftVolDownBtn, SIGNAL(pressed()), this, SLOT(leftVolDown()) );
    connect( mRightVolUpBtn, SIGNAL(pressed()), this, SLOT(rightVolUp()) );
    connect( mRightVolDownBtn, SIGNAL(pressed()), this, SLOT(rightVolDown()) );
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::openFile()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr("Open File"),
                                                     ".",
                                                     tr("Vidoes (*.mp4 *.mpg *.mpeg)"));
    DEBUG() << fileName;
    mMultimediaWidget->play( fileName );
}

void VideoPlayer::stop()
{
    mMultimediaWidget->stop();
}

void VideoPlayer::seek()
{
    int msec = 0;
    #define SEEK_NOYSE_WORKAROUND
    #ifdef SEEK_NOYSE_WORKAROUND
        static int workaround = 0;
        mMultimediaWidget->seek( msec );
        if ( workaround++ % 2 == 0 )
            QTimer::singleShot(50, this, SLOT(seek()));
    #else
        mMultimediaWidget->seek( msec );
    #endif
}

void VideoPlayer::pitchUp()      { mAudioEffect.mSemiTonesDelta += 1;  setAudioEffect(); }
void VideoPlayer::pitchDown()    { mAudioEffect.mSemiTonesDelta -= 1;  setAudioEffect(); }
void VideoPlayer::leftVolUp()    { mAudioEffect.mLeftChanVol += 0.05;  setAudioEffect(); }
void VideoPlayer::leftVolDown()  { mAudioEffect.mLeftChanVol -= 0.05;  setAudioEffect(); }
void VideoPlayer::rightVolUp()   { mAudioEffect.mRightChanVol += 0.05; setAudioEffect(); }
void VideoPlayer::rightVolDown() { mAudioEffect.mRightChanVol -= 0.05; setAudioEffect(); }
void VideoPlayer::setAudioEffect()
{
    mMultimediaWidget->setAudioEffect(
        mAudioEffect.mSemiTonesDelta,
        mAudioEffect.mLeftChanVol,
        mAudioEffect.mRightChanVol,
        false
        );
}
