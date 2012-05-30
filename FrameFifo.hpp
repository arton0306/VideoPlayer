#ifndef FRAME_FIFO_HPP
#define FRAME_FIFO_HPP

#include "UINT64_C_incpp.hpp"
extern "C"{
    #include "libavformat/avformat.h"
}

#include <queue>
#include <vector>

typedef unsigned char uint8;

class FrameFifo
{
public:
    FrameFifo();
    void push( vector<uint8> a_frame, double a_time );
    vector<uint8> pop();
    int getCount() const;
    double getMaxTime() const;
    double getFrontFrameTime() const;
    void clear();

private:
    std::queue<vector<uint8> > mFifo;
    std::queue<double> mTime;
    double mMaxTime;
};

#endif // FRAME_FIFO_HPP
