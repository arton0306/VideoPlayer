#include <stdexcept>
#include "PacketQueue.hpp"

PacketQueue::PacketQueue()
{
    init();
}

void PacketQueue::init()
{
    mFirstPacketNode = NULL;
    mLastPacketNode = NULL;
    mPacketsCount = 0;
    mSize = 0;
    mMutex = SDL_CreateMutex();
    mCond = SDL_CreateCond();
}

void PacketQueue::put( AVPacket * aPkt )
{
    ////////////////////////////////////////
    // Pack the AVPacket into AVPacketNode
    ////////////////////////////////////////

    AVPacketNode *packetNode;

    // don't know what this is ( ref: http://dranger.com/ffmpeg/tutorial03.html )
    if(av_dup_packet(aPkt) < 0)
        throw std::runtime_error( "av_dup_packet error" );

    packetNode = (AVPacketNode *)av_malloc(sizeof(AVPacketNode));
    if (!packetNode)
        throw std::runtime_error( "av_malloc error" );

    packetNode->pkt = *aPkt;
    packetNode->next = NULL;

    ////////////////////////////////////////
    // Put the AVPacketNode in the queue
    ////////////////////////////////////////

    SDL_LockMutex(mMutex);

    if ( mLastPacketNode == NULL )
        mFirstPacketNode = packetNode;
    else
        mLastPacketNode->next = packetNode;
    mLastPacketNode = packetNode;
    ++mPacketsCount;
    mSize += packetNode->pkt.size;
    SDL_CondSignal(mCond);

    SDL_UnlockMutex(mMutex);
}

AVPacket PacketQueue::get()
{
    AVPacket ret;
    AVPacketNode *tempPacketNode;

    SDL_LockMutex( mMutex );

    for(;;) {
        tempPacketNode = mFirstPacketNode;
        if ( mFirstPacketNode )
        {
            mFirstPacketNode = mFirstPacketNode->next;
            if ( mFirstPacketNode == NULL )
                mLastPacketNode = NULL;
            --mPacketsCount;
            mSize -= tempPacketNode->pkt.size;
            ret = tempPacketNode->pkt;
            av_free(tempPacketNode);
            break;
        }
        else
        {
            SDL_CondWait( mCond, mMutex );
        }
    }
    SDL_UnlockMutex( mMutex );
    return ret;
}

