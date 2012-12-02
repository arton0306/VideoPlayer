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
