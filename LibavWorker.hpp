#ifndef LIBAV_WORKER_HPP
#define LIBAV_WORKER_HPP

#include <vector>
#include <QObject>
#include <QString>
#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
}
#include "FrameFifo.hpp"
#include "AVInfo.hpp"
#include "AudioTuner.hpp"

class LibavWorker : public QObject
{
    Q_OBJECT
public:
    enum SeekResult
    {
        SEEK_SUCCESS,
        SEEK_FAIL
    };

    explicit LibavWorker(QObject *parent = 0);

public: // for other thread
    std::vector<uint8> popAllAudioStream();
    std::vector<uint8> popNextVideoFrame();
    void dropNextVideoFrame();
    double getNextVideoFrameSecond() const;
    void stopDecoding();

    // audio effect
    void setSpeechMode( bool aIsSpeechMode );
    void setPitchShiftInSemiTones( int aDelta /* -60 ~ +60 */ );
    void setVol( double aPercent /* 0.0 ~ 1.0 */ );
    void setLeftChanVol( double aPercent /* 0.0 ~ 1.0 */ );  // cannot be called if mono
    void setRightChanVol( double aPercent /* 0.0 ~ 1.0 */ ); // connot be called if mono

signals:
    void frameReady( uint8_t const * aPpmBuffer, int aPpmSize );
    void readyToDecode( AVInfo aAVInfo );
    void initAVFrameReady( double aFirstAudioFrameMsec );
    void decodeDone();
    void seekState( bool aResult );

public slots:
    void decodeAudioVideo( QString aFileName );
    void seek( int aMSec );

private:
    void init();
    std::vector<uint8> convertToUint8Stream( AVFrame *aDecodedFrame, int width, int height );
    void appendPcmToFile( void const * aPcmBuffer, int aPcmSize, char const * aFileName );
    void setFileName( QString aFileName );
    bool isAvFrameEnough( double a_fps ) const;

    // libav
    int readHeader( AVFormatContext ** aFormatCtx );
    int retrieveStreamInfo( AVFormatContext * aFormatCtx );
    int getStreamIndex( AVFormatContext * aFormatCtx, AVMediaType aMediaType );
    AVCodecContext * getCodecCtx( AVFormatContext * aFormatCtx, int aStreamIndex );
    void convertToRGBFrame( AVCodecContext * videoCodecCtx, AVFrame * decodedFrame, AVFrame * pFrameRGB );

    QString mFileName;
    FrameFifo mVideoFifo;
    FrameFifo mAudioFifo;
    bool mIsReceiveStopSignal; // if libav is in decoding loop, it will quit the loop when the flag be set true
    bool mIsReceiveSeekSignal;
    int mSeekMSec;             // other thread will set mIsReceiveSeekSignal and mSeekTime to notice libav thread
    bool mIsDecoding;

    // audio tuner ( sound touch )
    void setAudioEffect( int aChannel );

    AudioTuner mAudioTuner;
    bool mIsSpeechMode;
    int mSemiTonesDelta;
    double mLeftChanVol;  // if the audio is mono, mLeftChanVol == mRightChanVol
    double mRightChanVol;
};

#endif // LIBAV_WORKER_HPP
