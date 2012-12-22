# g++ -shared -o libfoo.dll foo.o -Wl,--out-implib,libfoo.a

all:
	g++ -c -fPIC foo.cpp -o foo.o
	g++ -shared -o libfoo.dll foo.o -Wl
	g++ -o using_foo.exe using_foo.cpp -L. -llibfoo
	
	g++ -c -fPIC wrap_foo.cpp -o wrap_foo.o
	g++ -shared -o libwrap_foo.dll wrap_foo.o -Wl -L. -llibfoo
	g++ -o using_wrap_foo.exe using_wrap_foo.cpp -L. -llibwrap_foo
