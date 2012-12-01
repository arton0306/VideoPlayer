#include <cassert>
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
    mMultimediaWidget->seek( 100000 );
}
