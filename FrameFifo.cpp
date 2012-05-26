#include <cassert>
#include "FrameFifo.hpp"

FrameFifo::FrameFifo()
{
}

void FrameFifo::push( FrameBuffer a_frame, double a_time )
{
    mFifo.push( a_frame );
    mTime.push( a_time );
    assert( a_time >= mMaxTime );
    mMaxTime = a_time;
}

FrameFifo::FrameBuffer FrameFifo::pop()
{
    FrameBuffer retFrame;
    if ( mFifo.size() > 0 )
    {
        retFrame = mFifo.front();
        mFifo.pop();
        mTime.pop();
    }
    return retFrame;
}

double FrameFifo::getFrontFrameTime() const
{
    return mTime.front();
}

int FrameFifo::getCount() const
{
    return mFifo.size();
}

double FrameFifo::getMaxTime() const
{
    return mMaxTime;
}
