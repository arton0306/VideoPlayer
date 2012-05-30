#ifndef LIBAV_WORKER_HPP
#define LIBAV_WORKER_HPP

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
    vector<uint8> popAllAudioStream();
    vector<uint8> popOneVideoFrame();

signals:
    void frameReady( uint8_t const * aPpmBuffer, int aPpmSize );
    void ready( AVInfo aAVInfo );

public slots:
    void decodeAudioVideo( QString aFileName );

private:
    void init();
    vector<uint8> convertToUint8Stream( AVFrame *aDecodedFrame, int width, int height );
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
    bool mIsReady; // true if the first time decoded phase is done
};

#endif // LIBAV_WORKER_HPP
