#include "../common.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <print>
#include <vector>
#include <random>
#include <atomic>
#include <array>
#include <algorithm>
#include <string_view>
#include <span>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27015

class MySocket
{
public:
    
    MySocket(u16 port = DEFAULT_PORT, u32 ip = INADDR_LOOPBACK)
    {
        int result;
        result = WSAStartup(MAKEWORD(2,2), &m_wsa);
        if (result != 0) {
            std::println("WSAStartup failed with error: {}", result);
            throw;
        }

        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_sock == INVALID_SOCKET)
        {
            std::println("socket function failed with error: {}", WSAGetLastError());
            throw;
        }
        
        sockaddr_in sockAddress
        {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr=
            {
                .S_un
                {
                    .S_addr = htonl(ip),
                }
            },
            .sin_zero = {}
        };

        if (connect(m_sock, reinterpret_cast<const sockaddr*>(&sockAddress), static_cast<int>(sizeof(sockAddress))) == SOCKET_ERROR)
        {
            std::println("connect function failed with error: {}", WSAGetLastError());
            throw;
        }
    }
    
    ~MySocket()
    {
        closesocket(m_sock);
        WSACleanup();
    }

    WSADATA m_wsa;
    SOCKET m_sock = INVALID_SOCKET;
};

struct Message
{
    Message(usize size) 
    {
        buffer.reserve(size);
    }
    
    std::vector<std::byte> buffer;
};

struct MessagePool
{
public:

    MessagePool(usize countOfMessages, usize preallocMsgSize)
    {
        m_pool.reserve(countOfMessages);
        m_free.reserve(countOfMessages);
        for (usize i{ 0 }; i < countOfMessages; ++i)
        {
            m_pool.emplace_back(preallocMsgSize);
            m_free.push_back(&m_pool.back());
        }
    }

    Message* Pop()
    {
        AcquireSRWLockExclusive(&m_srwLock);

        while(m_free.empty())
            SleepConditionVariableSRW(&m_cvAvailable, &m_srwLock, INFINITE, 0);

        Message* msg = m_free.back();
        m_free.pop_back();
        
        ReleaseSRWLockExclusive(&m_srwLock);
        
        return msg;
    }

    void Push(Message* msg)
    {
        AcquireSRWLockExclusive(&m_srwLock);
        
        m_free.push_back(msg);
        
        ReleaseSRWLockExclusive(&m_srwLock);
        WakeConditionVariable(&m_cvAvailable);
    }
    
private:

    SRWLOCK m_srwLock{ SRWLOCK_INIT };
    CONDITION_VARIABLE m_cvAvailable{ CONDITION_VARIABLE_INIT };
    std::vector<Message> m_pool;
    std::vector<Message*> m_free;
};

void GenerateMessage(Message* msg)
{
    auto lWriteU16 = [](Message* msg, usize offset,  u16 num) -> void
    {
        const u16 numBigEndian = (num << 8) | (num >> 8);
        msg->buffer[offset]     = static_cast<std::byte>((numBigEndian & 0x00FF));
        msg->buffer[offset + 1] = static_cast<std::byte>((numBigEndian >> 8) & 0x00FF);
    };

    static constexpr usize MSG_LIMIT{ 1 << 8 }; 
    thread_local std::mt19937 gen{ std::random_device{}() };
    std::uniform_int_distribution<usize> distrSize(0, MSG_LIMIT); 
    std::uniform_int_distribution<> distrValue(65, 90); 

    u16 sizeOfMsg{ static_cast<u16>(distrSize(gen)) }; 
    msg->buffer.resize(sizeOfMsg + sizeof(u16));

    lWriteU16(msg, 0, sizeOfMsg);
    
    for (usize n = sizeof(u16); n < sizeOfMsg + sizeof(u16); ++n)
    {
        msg->buffer[n] = static_cast<std::byte>(distrValue(gen));
    }
}


struct Context
{
    inline static constexpr usize MSG_COUNT = 1'000'000; 
    inline static MessagePool msgPool = MessagePool(100, 256); 
    inline static std::atomic<usize> messagesSent = 0;
    
    inline static constexpr usize GENERATOR_THREADS = 4; 
    inline static std::atomic<usize> threadsDone = 0;

    
    inline static constexpr usize MAX_BUFFER_SIZE = 10'000; 
    inline static std::vector<std::byte> sendingBuffer;
    inline static std::vector<std::byte> loadingBuffer;

    inline static SRWLOCK srwLoadingBuffer{ SRWLOCK_INIT };
    inline static CONDITION_VARIABLE m_cvAvailable{ CONDITION_VARIABLE_INIT };

    inline static std::atomic<usize> messagesThrown = 0;
    
    static void EnqueueForSend(Message* msg)
    {
        AcquireSRWLockExclusive(&srwLoadingBuffer);
        
        if (msg->buffer.size() + loadingBuffer.size() > MAX_BUFFER_SIZE)
        {
            messagesThrown++;
            ReleaseSRWLockExclusive(&srwLoadingBuffer);
            return;
        }

        loadingBuffer.insert(loadingBuffer.end(), msg->buffer.begin(), msg->buffer.end());
        
        ReleaseSRWLockExclusive(&srwLoadingBuffer);
    }

} g_context;

DWORD WINAPI GeneratorFunction([[maybe_unused]] LPVOID lpParam) 
{ 
    while (true)
    {
        if (Context::messagesSent.fetch_add(1) >= Context::MSG_COUNT)
            break;
        
        Message* msg = Context::msgPool.Pop();
        
        GenerateMessage(msg);
        Context::EnqueueForSend(msg);
        WakeConditionVariable(&Context::m_cvAvailable);
        Context::msgPool.Push(msg);
    }

    AcquireSRWLockExclusive(&Context::srwLoadingBuffer);
    ++Context::threadsDone;
    WakeConditionVariable(&Context::m_cvAvailable);
    ReleaseSRWLockExclusive(&Context::srwLoadingBuffer);
    
    return 0; 
} 

void SendAll(SOCKET s, std::span<const std::byte> data)
{
    usize sent = 0;
    while (sent < data.size())
    {
        const int chunk = static_cast<int>(std::min<usize>(data.size() - sent, 64 * 1024));
        const int r = send(s, reinterpret_cast<const char*>(data.data() + sent), chunk, 0);
        if (r == SOCKET_ERROR) { std::println("send failed: {}", WSAGetLastError()); throw; }
        sent += static_cast<usize>(r);
    }
}

int main() 
{
    MySocket socket;
    int result{};

    Context::loadingBuffer.reserve(Context::MAX_BUFFER_SIZE);
    Context::sendingBuffer.reserve(Context::MAX_BUFFER_SIZE);

    constexpr std::string_view text{"Hello message from client."};
    std::vector<std::byte> hello;
    hello.push_back(std::byte(text.size() >> 8));
    hello.push_back(std::byte(text.size() & 0xFF));
    auto p = reinterpret_cast<const std::byte*>(text.data());
    hello.insert(hello.end(), p, p + text.size());
    SendAll(socket.m_sock, hello);
    
    std::array<HANDLE, Context::GENERATOR_THREADS> threads;

    for (auto& thread : threads)
    {
        thread = CreateThread(
            nullptr /* default security attributes */ , 
            0       /* default stack size  */,
            (LPTHREAD_START_ROUTINE)GeneratorFunction /* thread function */,
            nullptr /* thread context */, 
            0       /* flags */,
            nullptr /* threadId return */
        );
    }
    
    bool done = false;
    while (!done)
    {
        SendAll(socket.m_sock, Context::sendingBuffer);   // only the sender touches sendingBuffer, so no lock
        Context::sendingBuffer.clear();
    
        AcquireSRWLockExclusive(&Context::srwLoadingBuffer);
        while (Context::loadingBuffer.empty() && Context::threadsDone != Context::GENERATOR_THREADS)
            SleepConditionVariableSRW(&Context::m_cvAvailable, &Context::srwLoadingBuffer, INFINITE, 0);
    
        std::swap(Context::loadingBuffer, Context::sendingBuffer);
        done = Context::sendingBuffer.empty() && Context::threadsDone == Context::GENERATOR_THREADS;
        ReleaseSRWLockExclusive(&Context::srwLoadingBuffer);
    }

    WaitForMultipleObjects(
        threads.size(), 
        threads.data(), 
        TRUE, 
        INFINITE
    );
    
    for (auto& thread : threads)
    {
        CloseHandle(thread);
    }

    shutdown(socket.m_sock, SD_SEND);
    
    // 2. Read and discard whatever the server sent (its hello) until it closes too.
    char drain[256];
    while (recv(socket.m_sock, drain, sizeof(drain), 0) > 0) {}

    
    std::println("Messages thrown: {}", Context::messagesThrown.load());
    
    return 0;

}
