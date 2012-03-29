#include <stdexcept>
#include "PacketQueue.hpp"

PacketQueue::PacketQueue()
{
    // mMutex = SDL_CreateMutex();
    // mCond = SDL_CreateCond();
}

void PacketQueue::put( AVPacket * aPkt )
{
    if(av_dup_packet(aPkt) < 0)
        throw std::runtime_error( "av_dup_packet error" );

    // SDL_LockMutex(mMutex);
    mQueue.push( *aPkt );
    mSize += aPkt->size;
    // SDL_CondSignal(mCond);
    // SDL_UnlockMutex(mMutex);
}

AVPacket PacketQueue::get()
{
    AVPacket ret;
    // SDL_LockMutex( mMutex );
    for(;;) {
        if ( !mQueue.empty() )
        {
            ret = mQueue.front();
            mQueue.pop();
            mSize -= ret.size;
            break;
        }
        else
        {
            // SDL_CondWait( mCond, mMutex );
        }
    }
    // SDL_UnlockMutex( mMutex );
    return ret;
}

