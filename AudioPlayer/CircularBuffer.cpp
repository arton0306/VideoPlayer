#include <algorithm>
#include <cassert>
#include "CircularBuffer.hpp"

#define uint8_t char

CirBuf::CirBuf(int cap)
{
    assert(cap >= 1024);
    mCap = cap;
    mBuf = new uint8_t[cap + 1];
    mStart = mEnd = 0;
}

CirBuf::~CirBuf()
{
    delete mBuf;
}

int CirBuf::addData(char const * buf, int len)
{
    int rooms = getCap() - getUsed();
    int addLen = std::min(len, rooms);
    for (int i = 0; i < addLen; i++) {
        addByte(buf[i]);
    }
    return addLen;
}

bool CirBuf::addByte(uint8_t byte)
{
    mBuf[mEnd] = byte;
    mEnd++;
    if (mEnd > mCap) {
        mEnd = 0;
    }
}

int CirBuf::getUsed() const
{
    if (mStart <= mEnd) {
        return mEnd - mStart;
    } else {
        return (mCap - mStart + 1) + mEnd;
    }
}

bool CirBuf::isFull() const
{
    return getUsed() == mCap;
}

bool CirBuf::isEmpty() const
{
    return mEnd == mStart;
}

char CirBuf::pop()
{
    assert(getUsed() > 0);
    char b = mBuf[mStart];
    mStart++;
    if (mStart > mCap) {
        mStart = 0;
    }
    return b;
}
