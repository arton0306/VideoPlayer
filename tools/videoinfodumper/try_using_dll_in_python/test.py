from ctypes import cdll
lib = cdll.libfoo
# or
# lib = cdll.LoadLibrary('./libfoo.dll')

class Foo(object):
    def __init__(self):
        self.obj = lib.Foo_new()

    def bar(self):
        lib.Foo_bar(self.obj)

if __name__ == '__main__':
    f = Foo()
    f.bar() #and you will see "Hello" on the screen
