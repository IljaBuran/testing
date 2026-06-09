#include "common.h"

#include <print>

class Foo
{
public:


    Foo(i32 val)
        : value(val) {}

    i32 Add(i32 a) { return a + value; }

private:

    i32 value = 0;;
};


i32 main()
{
    Foo foo5(5);
    Foo foo10(10);

    int(Foo::*addFnPtr)(i32) = &Foo::Add;

    int result5 = (foo5.*addFnPtr)(5);
    int result10 = (foo10.*addFnPtr)(5);

    std::println("{}", result5);
    std::println("{}", result10);

    return 0;
}