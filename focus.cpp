#include <iostream>
#include <vector>
#include <utility>


struct S
{
    S()
    {
        std::cout << "Constructor Called\n";
    }

    S(const S& s)
    {
        std::cout << "Copy Constructor Called\n";
    }

    S(S&& s)
    {
        std::cout << "Move Constructor Called\n";
    }

    void operator=(const S& other)
    {
        std::cout << "Operator= Called";
    }

};

int main()
{
    std::vector<S> vectorOfS;

    vectorOfS.push_back(S());

    return 0;
}
