#ifndef LIBAVWORKER_HPP
#define LIBAVWORKER_HPP

#include <QObject>
#include <QString>
#include "PacketQueue.hpp"
#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
}

class LibavWorker : public QObject
{
    Q_OBJECT
public:
    explicit LibavWorker(QObject *parent = 0);

signals:
    void frameReady( uint8_t const * aPpmBuffer, int aPpmSize );

public slots:
    void decodeAudioVideo( QString aFileName );
    uint8_t const * getPpmBuffer() const;
    int getPpmSize() const;

private:
    void saveFrame( int aFrame );
    void fillPpmBuffer( AVFrame *pFrame, int width, int height );
    void appendPcmToFile( void const * aPcmBuffer, int aPcmSize, char const * aFileName );

    void setFileName( QString aFileName );
    int readHeader( AVFormatContext ** aFormatCtx );
    int retrieveStreamInfo( AVFormatContext * aFormatCtx );
    int getStreamIndex( AVFormatContext * aFormatCtx, AVMediaType aMediaType );
    AVCodecContext * getCodecCtx( AVFormatContext * aFormatCtx, int aStreamIndex );
    void convertToRGBFrame( AVCodecContext * videoCodecCtx, AVFrame * decodedFrame, AVFrame * pFrameRGB );

    QString mFileName;

    // 100 for ppm file header, 3 for RGB, 1920 * 1680 for max screen
    uint8_t mPpmBuffer[100+3*1920*1680];
    int mPpmSize;
    PacketQueue mAudioQueue;
};

#endif // LABAVWORKER_H
