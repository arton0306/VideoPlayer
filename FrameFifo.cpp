#include <cassert>
#include "FrameFifo.hpp"

using namespace std;

FrameFifo::FrameFifo()
    : mMaxTime( 0.0 )
{
}

void FrameFifo::push( vector<uint8> a_frame, double a_time )
{
    mMutex.lock();
    {
        mFifo.push( a_frame );
        mTime.push( a_time );
        assert( a_time >= mMaxTime );
        mMaxTime = a_time;
    }
    mMutex.unlock();
}

vector<uint8> FrameFifo::pop()
{
    vector<uint8> retFrame;
    mMutex.lock();
    {
        if ( mFifo.size() > 0 )
        {
            retFrame = mFifo.front();
            mFifo.pop();
            mTime.pop();
        }
    }
    mMutex.unlock();
    return retFrame;
}

double FrameFifo::getFrontFrameTime() const
{
    double result = 0.0;
    mMutex.lock();
    {
        if ( !mTime.empty() )
        {
            result = mTime.front();
        }
    }
    mMutex.unlock();
    return result;
}

int FrameFifo::getCount() const
{
    int result = 0;
    mMutex.lock();
    {
        result = mFifo.size();
    }
    mMutex.unlock();
    return result;
}

double FrameFifo::getMaxTime() const
{
    return mMaxTime;
}

void FrameFifo::clear()
{
    mMutex.lock();
    {
        assert( mFifo.size() == mTime.size() );
        while ( !mFifo.empty() )
        {
            mFifo.pop();
            mTime.pop();
        }
        mMaxTime = 0.0;
    }
    mMutex.unlock();
}
