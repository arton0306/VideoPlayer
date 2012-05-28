AVInfo::AVInfo( double aFps )
    : mFps( aFps )
{}

double AVInfo::getFps() const
{
    return mFps;
}

double getLenth() const
{
    return mLength;
}

AVInfo( AVInfo const & aAvInfo )
{
    mFps = aAvInfo.mFps;
    mLength = aAvInfo.mLength;
}
