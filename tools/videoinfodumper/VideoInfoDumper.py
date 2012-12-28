#-*- encoding: cp950 -*-
import os
import sys
import StringIO
from ctypes import cdll
lib = cdll.libavinfodumper

def getExtName( fileName ):
    if ( "." in fileName ):
        return fileName[fileName.rindex( os.extsep ) + 1:]
    else:
        return None

def isVideoFile( fileName ):
    videoExtName = (
        "dat",
        "mp4",
        "mpg",
        "mov",
        "mpeg",
        "mp4",
        "avi",
        "mp4",
        )
    extName = getExtName( fileName )
    return extName and extName.lower() in videoExtName

if __name__ == '__main__':
    # lib.libav_dumper always dump info to cerr
    # we let python print redirect to stderr
    sys.stdout = sys.stderr

    path = "G:\KTV\整理好的歌"
    """
    for filename in os.listdir( path ):
        fullname = path + os.sep + filename
        print "#" * 70, fullname
        if ( isVideoFile( filename ) ):
            #print( "=" * 70 )
            lib.libav_dumper( fullname )
            print ""
    """
    #lib.libav_dumper( "G:\KTV\整理好的歌\國-女-一青窈-望春風.mpg" )
    #lib.libav_dumper( "G:\KTV\整理好的歌\國-女-丁噹-代替.mpg" )
    lib.libav_dumper( "G:\KTV\整理好的歌\台-男-陳雷-最佳男主角.mpg" )
