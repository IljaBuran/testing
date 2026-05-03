#include <iostream>
#include <thread>
#include <atomic>
using i32 = int;


int main()
{
    std::atomic<i32> counter = 0;
    auto work = [&counter]()
    {
        for (i32 i = 0; i < 10'000; i++)
        counter++;
    };


    std::thread t1(work);
    std::thread t2(work);

    t1.join();
    t2.join();

    std::cout << counter << '\n';

    return 0;
}