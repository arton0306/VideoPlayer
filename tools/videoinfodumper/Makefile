CC = g++

ADD_INC_FOLDER = -I../../libav/include

ADD_LIB_FOLDER = -L../../libav/lib

ADD_LIB =       \
    -lavcodec   \
    -lavdevice  \
    -lavfilter  \
    -lavformat  \
    -lavutil    \
    -lswscale

build-all: build-dumper-exe build-dll build-dumper-dll-test

build-dumper-exe:
	$(CC) dumper.cpp -o dumper.exe $(ADD_INC_FOLDER) $(ADD_LIB_FOLDER) $(ADD_LIB)

build-dll:
	$(CC) -c -fPIC dumper.cpp -o dumper.o $(ADD_INC_FOLDER) -DBUILD_DLL
	$(CC) -shared -o libavinfodumper.dll dumper.o -Wl,$(ADD_LIB_FOLDER) $(ADD_LIB)

build-dumper-dll-test:
	$(CC) using_dumper.cpp -o dumper_dll_test.exe -L. -llibavinfodumper
