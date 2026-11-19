#include "common.hpp"
#include <Windows.h>
#include <vector>
#include <print>

struct ThreadContext
{
    volatile inline static u64 var;
};

constexpr static usize ITERATIONS{ 1'000'000 };

DWORD WINAPI ThreadFunction(
    [[maybe_unused]] LPVOID parameter
)
{
    for (usize i{ 0 }; i < ITERATIONS; ++i)
    {
        ThreadContext::var++;
    }
    return 0;
}

int main()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    const DWORD numCPU = sysinfo.dwNumberOfProcessors;
    std::vector<HANDLE> threadHandles(numCPU);

    std::println("Creating {} threads.", numCPU);
    
    for (auto& handle : threadHandles)
    {
        handle = CreateThread(
            nullptr /* default security attributes */ , 
            0       /* default stack size  */,
            (LPTHREAD_START_ROUTINE)ThreadFunction /* thread function */,
            nullptr /* thread context */, 
            0       /* flags */,
            nullptr /* threadId return */
        );
    }

    WaitForMultipleObjects(
        numCPU, 
        threadHandles.data(), 
        TRUE, 
        INFINITE
    );

    for (auto& handle : threadHandles)
    {
        CloseHandle(handle);
    }

    u64 v = ThreadContext::var;
    
    std::print("Result: {}", v);
    return 0;
}
