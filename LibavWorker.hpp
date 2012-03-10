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

private:
    void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);
    int libav();
};

#endif // LABAVWORKER_H
