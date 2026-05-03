#include <iostream>

std::string s;
#if defined(__clang__)
    static std::string print()
    {
        return "clang";
    }
#elif defined(__GNUC__)
    static std::string print()
    {
        return "clang";
    }
#endif


int main()
{
    std::cout << print() << '\n';
    return 0;
}   