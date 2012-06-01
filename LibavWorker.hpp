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

class LibavWorker : public QObject
{
    Q_OBJECT
public:
    explicit LibavWorker(QObject *parent = 0);
    std::vector<uint8> popAllAudioStream();
    std::vector<uint8> popNextVideoFrame();
    void dropNextVideoFrame();
    double getNextVideoFrameSecond() const;
    void setCurrentPlaySecond( double a_time );

signals:
    void frameReady( uint8_t const * aPpmBuffer, int aPpmSize );
    void ready( AVInfo aAVInfo );

public slots:
    void decodeAudioVideo( QString aFileName );

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
    double mCurrentPlaySecond;
};

#endif // LIBAV_WORKER_HPP
