#include <cassert>
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
}

VideoPlayer::~VideoPlayer()
{
}
