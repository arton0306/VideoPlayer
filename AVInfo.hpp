#ifndef AV_INFO_HPP
#define AV_INFO_HPP

class AVInfo
{
public:
    AVInfo( double aFps );
    AVInfo( AVInfo const & aAvInfo );
    double getFps() const;
    double getLenth() const;

private:
    double mFps;
    double mLength;
};

#endif // AV_INFO_HPP
