# g++ -shared -o libfoo.dll foo.o -Wl,--out-implib,libfoo.a

all:
	g++ -c -fPIC foo.cpp -o foo.o
	g++ -shared -o libfoo.dll foo.o -Wl
	g++ -o using_dll.exe test.cpp -L. -llibfoo

nofpic:
	g++ -c foo.cpp -o foo.o
	g++ -shared -o libfoo.dll foo.o -Wl
	g++ -o using_dll.exe test.cpp -L. -llibfoo
