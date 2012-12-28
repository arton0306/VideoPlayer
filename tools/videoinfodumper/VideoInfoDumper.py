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

    path = "G:\KTV\��z�n���q"
    """
    for filename in os.listdir( path ):
        fullname = path + os.sep + filename
        print "#" * 70, fullname
        if ( isVideoFile( filename ) ):
            #print( "=" * 70 )
            lib.libav_dumper( fullname )
            print ""
    """
    #lib.libav_dumper( "G:\KTV\��z�n���q\��-�k-�@�C��-��K��.mpg" )
    #lib.libav_dumper( "G:\KTV\��z�n���q\��-�k-�B��-�N��.mpg" )
    lib.libav_dumper( "G:\KTV\��z�n���q\�x-�k-���p-�̨Ψk�D��.mpg" )
