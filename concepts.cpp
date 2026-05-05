#include "common.h"

#include <iostream>
#include <concepts>
#include <vector>

template<typename T>
concept Printable = requires(std::ostream& os, const T& a)
{
    { os << a } -> std::same_as<std::ostream&>;
};

template<Printable T>
void Print(const T& t)
{
    std::cout << t << '\n';
}

void TestPrint()
{
    i32 i = 312;
    std::vector<i32> vec = {1, 2, 3};

    Print(i);
    // Print(vec); /* compilation should fail here */
}

template<typename T>
concept Addable = requires(const T& a, const T& b)
{
    { a + b } -> std::same_as<T>;
};


template<Addable T>
T Add(const T& a, const T& b)
{
    return a + b;
}

/* Ill define my string on top of std::string with operator++ */
class MyString : public std::string
{
public:
    using std::string::string;

    MyString& operator++()
    {
        (*this)[0] = '0';
        return *this;
    }
    MyString operator++(i32)
    {
        MyString ret = (*this);
        (*this)[1] = '0';
        return ret;
    }
};

template<typename T>
concept PreIncrementable = requires(T& a)
{
    { ++a } -> std::same_as<T&>;
};


template<PreIncrementable T>
T PreIncrement(T& a)
{
    return ++a;
}

template<typename T>
concept PostIncrementable = requires(T& a)
{
    { a++ } -> std::same_as<T>;
};


template<PostIncrementable T>
T PostIncrement(T& a)
{
    return a++;
}


void TestIncrement()
{
    i32 i1 = 312;
    std::vector<i32> vec = {1, 2, 3};

    MyString s1 = "Hello, World!";

    // std::cout << i1 << '\n';
    // PreIncrement(i1);
    // std::cout << i1 << '\n';

    Print(PreIncrement(i1));
    Print(PreIncrement(s1));

    Print(PostIncrement(i1));
    Print(PostIncrement(s1));

    Print(s1);
    // Print(Add(cs1, cs2));
}

i32 main()
{
    // TestPrint();
    // TestAdd();
    TestIncrement();

    return 0;
}