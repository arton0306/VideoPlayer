#ifndef FRAME_FIFO_HPP
#define FRAME_FIFO_HPP

#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavformat/avformat.h"
}

#include <queue>
#include <vector>

class FrameFifo
{
public:
    typedef std::vector<char> FrameBuffer;

    FrameFifo();
    void push( FrameBuffer a_frame, double a_time );
    FrameBuffer pop();
    int getCount() const;
    double getMaxTime() const;
    double getFrontFrameTime() const;

private:
    std::queue<FrameBuffer> mFifo;
    std::queue<double> mTime;
    double mMaxTime;
};

#endif // FRAME_FIFO_HPP
