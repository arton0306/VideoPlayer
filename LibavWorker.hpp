#ifndef LIBAVWORKER_HPP
#define LIBAVWORKER_HPP

#include <QObject>

extern "C"{
#ifdef __cplusplus
    #define __STDC_CONSTANT_MACROS
    #ifdef _STDINT_H
        #undef _STDINT_H
    #endif
    #include <stdint.h>
#endif
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
    
public slots:
    void doWork();
    
    char const * getPpmBuffer() const;
    int getPpmSize() const;

private:
    void saveFrame(AVFrame *pFrame, int width, int height, int iFrame);
    void saveFrame( int aFrame );
    void fillPpmBuffer( AVFrame *pFrame, int width, int height );
    int libav();

    // 100 for ppm file header, 3 for RGB, 1920 * 1680 for max screen
    char mPpmBuffer[100+3*1920*1680];
    int mPpmSize;
};

#endif // LABAVWORKER_H
