#ifndef LIBAVWORKER_HPP
#define LIBAVWORKER_HPP

#include <QObject>
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
    void doWork();
    
    uint8_t const * getPpmBuffer() const;
    int getPpmSize() const;

private:
    void saveFrame(AVFrame *pFrame, int width, int height, int iFrame);
    void saveFrame( int aFrame );
    void fillPpmBuffer( AVFrame *pFrame, int width, int height );
    int libav();

    // 100 for ppm file header, 3 for RGB, 1920 * 1680 for max screen
    uint8_t mPpmBuffer[100+3*1920*1680];
    int mPpmSize;
};

#endif // LABAVWORKER_H
