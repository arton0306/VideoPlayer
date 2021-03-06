#include <cmath> // for abs, there are a int version abs in cstdlib, we must include cmath to overwrite it
#include <QThread>
#include <cassert>
#include "MultimediaWidget.hpp"
#include "debug.hpp"
#include "DecodeWorker.hpp"
#include "QGLCanvas.hpp"

using namespace std;

MultimediaWidget::MultimediaWidget(QWidget *parent)
    : QWidget(parent)
    , mLibavWorker( new DecodeWorker )
    , mVideoCanvas( NULL )
    , mVolumnTuner( NULL )
    , mAudioPlayer( NULL )
    , mAudioSeekTimeMSec( 0.0 )
    , mIsDecodeDone( false )
{
    setupUi(this);
    setupConnection();

    AudioPlayer::init();

    mVideoCanvas = new QGLCanvas( this );
    mMainLayout->addWidget( mVideoCanvas );

    QThread * libavThread = new QThread;
    mLibavWorker->moveToThread( libavThread );
    libavThread->start();
    if (0) {
        play( QString( "./lulu.mp4" ) );
        //play( QString( "./1000km.mpg" ) );
    }
}

void MultimediaWidget::setupConnection()
{
    connect( mLibavWorker, SIGNAL(readyToDecode( AVInfo )), this, SLOT(getReadyToDecodeSignal( AVInfo )) );
    connect( mLibavWorker, SIGNAL(decodeDone()), this, SLOT(getDecodeDoneSignal()) );
    connect( mLibavWorker, SIGNAL(seekState(bool)), this, SLOT(getSeekStateSignal(bool)) );
    connect( mLibavWorker, SIGNAL(initAVFrameReady(double)), this, SLOT(getInitAVFrameReadySignal(double)) );
    connect( &mTimer, SIGNAL(timeout()), this, SLOT(updateAV()) );
}

void MultimediaWidget::getReadyToDecodeSignal( AVInfo aAvInfo )
{
    // TODO: supported format check
    mCurrentAvInfo = aAvInfo;
    mCurrentAvInfo.dump();


    mAudioSeekTimeMSec = 0;
    delete mAudioPlayer;
    mAudioPlayer = new AudioPlayer( aAvInfo.getAudioChannel(), aAvInfo.getAudioSampleFormat(), aAvInfo.getAudioSampleRate() );
    delete mVolumnTuner;
    mVolumnTuner = new VolumeTuner( aAvInfo.getAudioChannel(), aAvInfo.getAudioSampleFormat() );
}

void MultimediaWidget::getSeekStateSignal(bool aIsSuccess)
{
    DEBUG() << " =========================== seek result: " << aIsSuccess;
    if ( aIsSuccess ) // true is success
    {
        mTimer.stop();
        if ( mAudioPlayer ) mAudioPlayer->stop();
        mAudioStreamBuffer.clear();
    }
}

void MultimediaWidget::getInitAVFrameReadySignal(double aFirstAudioFrameMsec)
{
    mAudioSeekTimeMSec = aFirstAudioFrameMsec;
    DEBUG() << "libav seek from (ms): " << aFirstAudioFrameMsec;
    fetchAllAvailableAudioAndPush();
    mAudioPlayer->play();
    mTimer.start( getRenewPeriod() );
}

double MultimediaWidget::getRenewPeriod() const
{
    return 5;
}

double MultimediaWidget::getAudioPlayedSecond() const
{
    // we use qt audio device in the old version,
    // the played-time got by Qt lib has about 40ms error
    // so we use a outside clock to sync audio&video, but we adjust the outside clock to match the played-time(by Qt) if the gap become too large ( >sAudioAdjustGapMs )

    // now, we use portaudio, remove that workaround
    // ( but plyaing-time in portaudio still has buffer latency, if frames_per_buffer is 12xx, in 48000Hz audio, the letency is about 25ms )

    // DEBUG() << "get playing time: " << mAudioSeekTimeMSec / 1000.0 + mAudioPlayer->getPlaySec();
    return mAudioSeekTimeMSec / 1000.0 + mAudioPlayer->getPlaySec();
}

int MultimediaWidget::pushAudioStream( vector<uint8> const & stream ) const
{
    return mAudioPlayer->pushStream( reinterpret_cast<const char *>( &stream[0] ), stream.size() );
}

void MultimediaWidget::fetchAllAvailableAudioAndPush()
{
    vector<uint8> audioStreamFetched = mLibavWorker->popAllAudioStream();
    mVolumnTuner->tune( audioStreamFetched );
    int const writeBytes = pushAudioStream( audioStreamFetched );
    mAudioStreamBuffer.insert( mAudioStreamBuffer.end(), audioStreamFetched.begin() + writeBytes, audioStreamFetched.end() );
}


void MultimediaWidget::updateAV()
{
    if ( mAudioStreamBuffer.empty() )
    {
        fetchAllAvailableAudioAndPush();
    }
    else
    {
        int const writeBytes = pushAudioStream( mAudioStreamBuffer );
        mAudioStreamBuffer.erase( mAudioStreamBuffer.begin(), mAudioStreamBuffer.begin() + writeBytes );
    }

    // get the time of audio played
    double const currentPlaySecond = getAudioPlayedSecond();

    // get the time of the next video frame in decoded buffer
    double const nextVideoFrameSecond = mLibavWorker->getNextVideoFrameSecond();
    if ( nextVideoFrameSecond >= 0.0 )
    {
        // updateAV video frame if needed
        double const diff = currentPlaySecond - nextVideoFrameSecond;

        if ( std::abs(diff) < 0.015 )
        {
            vector<uint8> videoFrameStream = mLibavWorker->popNextVideoFrame();
            DEBUG() << "frame be drawed on the canvas, its exact time is:" << nextVideoFrameSecond << "\t currentSound:" << currentPlaySecond << "abs_diff: " << abs(diff);
            mVideoCanvas->renewFrame( static_cast<uint8_t const *>( &videoFrameStream[0] ), videoFrameStream.size() );
        }
        else if ( diff > 0.015 )
        {
            // video frame too old
            DEBUG() << "drop a frame which should be presented at:" << nextVideoFrameSecond << "\t currentSound:" << currentPlaySecond;
            mLibavWorker->dropNextVideoFrame();
        }

        // DEBUG() << "next video frame:" << nextVideoFrameSecond << "\t currentSound:" << currentPlaySecond;
    }
    else // there are no frame got
    {
        DEBUG() << "there are no video frame";
        if ( mIsDecodeDone )
        {
            stop();
        }
    }
}

QAudioFormat MultimediaWidget::getAudioFormat( AVInfo const & aAvInfo ) const
{
    QAudioFormat audioFormat;
    audioFormat.setSampleRate( aAvInfo.getAudioSampleRate() );
    audioFormat.setChannelCount( aAvInfo.getAudioChannel() );
    audioFormat.setSampleSize( aAvInfo.getAudioBitsPerSample() );
    audioFormat.setCodec( "audio/pcm" );
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::UnSignedInt);

    return audioFormat;
}

MultimediaWidget::~MultimediaWidget()
{
    AudioPlayer::finish();
}

void MultimediaWidget::play( QString aFileName )
{
    stop();
    QMetaObject::invokeMethod( mLibavWorker, "decodeAudioVideo", Qt::QueuedConnection,
        Q_ARG(QString, aFileName) );
}

void MultimediaWidget::seek( int aMSec )
{
    DEBUG() << "someone click seek";
    mLibavWorker->seek( aMSec );
}

void MultimediaWidget::stop()
{
    mTimer.stop();
    mVideoCanvas->clear();
    mAudioStreamBuffer.clear();
    if ( mAudioPlayer ) mAudioPlayer->stop(); // at starting, it is null
    mLibavWorker->stopDecoding();
    mAudioSeekTimeMSec = 0.0;
    // TODO: using portaudio
}

void MultimediaWidget::dumpAVStream()
{
    
}

void MultimediaWidget::getDecodeDoneSignal()
{
    mIsDecodeDone = true;
}

void MultimediaWidget::setAudioEffect( int pitchDelta, double leftVol, double rightVol, bool speechMode ) const
{
    mLibavWorker->setPitchSemiTones( pitchDelta );
    mLibavWorker->setSpeechMode( speechMode );

    mVolumnTuner->setLeftVol( leftVol );
    mVolumnTuner->setRightVol( rightVol );
}
