#include "foo.hpp"

extern "C" {
    void __declspec(dllexport) wrap()
    {
        Foo foo;
        foo.bar();
    }
}
