#include <iostream>
#include "foo.hpp"

void Foo::bar()
{
    std::cout << "Hello" << std::endl;
}

extern "C" {
    __declspec(dllexport) Foo* Foo_new(){ return new Foo(); }
    __declspec(dllexport) void Foo_bar(Foo* foo){ foo->bar(); }
}
