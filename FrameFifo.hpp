#ifndef FRAME_FIFO_HPP
#define FRAME_FIFO_HPP

#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavformat/avformat.h"
}

#include <queue>
#include <vector>
#include <QMutex>

typedef unsigned char uint8;

class FrameFifo
{
public:
    FrameFifo();
    void push( std::vector<uint8> a_frame, double a_time );
    std::vector<uint8> pop();
    int getCount() const;
    bool isEmpty() const;
    double getMaxTime() const;  // not used for the time being
    double getFrontFrameSecond() const;
    void clear();
    int getBytes() const;

private:
    std::queue<std::vector<uint8> > mFifo;
    std::queue<double> mTime; // in sec
    double mMaxTime; // in sec
    mutable QMutex mMutex;
    int mBytes;
};

#endif // FRAME_FIFO_HPP
