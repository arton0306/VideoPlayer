from ctypes import cdll
lib = cdll.libwrap_foo
# or
# lib = cdll.LoadLibrary('./libfoo.dll')

if __name__ == '__main__':
    lib.wrap() #and you will see "Hello" on the screen
