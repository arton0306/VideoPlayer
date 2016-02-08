#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

class CirBuf
{
public:
    CirBuf(int cap);
    ~CirBuf();

    int addData(char const * buf, int len);
    char pop();
    int getUsed() const;
    bool isFull() const;
    bool isEmpty() const;
    int getCap() const { return mCap; };

private:
    bool addByte(char byte);
    int  mCap;       // the max byte count which the mBuf can contain
    int  mStart;     // read from the index in the buffer
    int  mEnd;       // write to the index in the buffer, element on this index is always empty
    char *mBuf;      // the alloc size is mCap + 1
};

#endif
