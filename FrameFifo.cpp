#include <cassert>
#include "FrameFifo.hpp"

using namespace std;

FrameFifo::FrameFifo()
{
    mMaxTime = 0.0;
    mBytes = 0;
}

void FrameFifo::push( vector<uint8> a_frame, double a_time )
{
    mMutex.lock();
    {
        mFifo.push( a_frame );
        mTime.push( a_time );
        assert( a_time >= mMaxTime );
        mMaxTime = a_time;
        mBytes += a_frame.size();
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
            mBytes -= retFrame.size();
        }
    }
    mMutex.unlock();
    return retFrame;
}

// return negtive if the fifo is empty
double FrameFifo::getFrontFrameSecond() const
{
    double result = -100.0;
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

bool FrameFifo::isEmpty() const
{
    return getCount() == 0;
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
        mBytes = 0;
    }
    mMutex.unlock();
}

int FrameFifo::getBytes() const
{
    return mBytes;
}

