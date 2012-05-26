#ifndef FRAME_FIFO_HPP
#define FRAME_FIFO_HPP

#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavformat/avformat.h"
}

#include <queue>

class FrameFifo
{
public:
    FrameFifo();
    void put( AVPacket * aPkt );
    AVPacket get();

private:
    std::queue<AVPacket> mQueue;
    int mSize;
};

#endif // FRAME_FIFO_HPP
