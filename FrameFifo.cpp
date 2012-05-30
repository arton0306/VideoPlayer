#include <cassert>
#include "FrameFifo.hpp"

FrameFifo::FrameFifo()
    : mMaxTime( 0.0 )
{
}

void FrameFifo::push( vector<uint8> a_frame, double a_time )
{
    mFifo.push( a_frame );
    mTime.push( a_time );
    assert( a_time >= mMaxTime );
    mMaxTime = a_time;
}

vector<uint8> FrameFifo::pop()
{
    vector<uint8> retFrame;
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

void FrameFifo::clear()
{
    assert( mFifo.size() == mTime.pop() );
    while ( !mFifo.empty() )
    {
        mFifo.pop();
        mTime.pop();
    }
    mMaxTime = 0.0;
}
