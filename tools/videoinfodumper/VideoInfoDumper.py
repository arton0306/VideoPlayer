from ctypes import cdll
lib = cdll.libavinfodumper

if __name__ == '__main__':
    lib.libav_dumper( "Lelouch.mp4" );
