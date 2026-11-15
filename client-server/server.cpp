#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../common.hpp"

#include <print>
#include <cstring>
#include <string_view>
#include <span>
#include <array>
#include <vector>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27015

class MySocket
{
public:
    
    MySocket(SOCKET* client, u16 port = DEFAULT_PORT, u32 ip = INADDR_LOOPBACK)
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
        result = bind(m_sock, reinterpret_cast<SOCKADDR*>(&sockAddress), sizeof(sockAddress));
        if (result == SOCKET_ERROR)
        {
            std::println("bind function failed with error: {}", WSAGetLastError());
            throw;
        }
    
        result = listen(m_sock, SOMAXCONN);
        if (result == SOCKET_ERROR)
        {
            std::println("listen function failed with error: {}", WSAGetLastError());
            throw;
        }
        
        // Accept a client socket
        *client = accept(m_sock, NULL, NULL);
        if (*client == INVALID_SOCKET)
        {
            std::println("accept function failed with error: {}", WSAGetLastError());
            throw;
        }

        constexpr std::string_view helloMessage{"Hello message from server."};
        usize sent{ 0 };
        while (sent != helloMessage.size())
        {
            result = send(*client, helloMessage.data() + sent, helloMessage.size() - sent, 0);
            if (result == SOCKET_ERROR)
            {
                std::print("send failed with error: {}\n", WSAGetLastError());
                throw;
            }
            sent += static_cast<usize>(result);
        }

        result = shutdown(*client, SD_SEND);
        if (result == SOCKET_ERROR)
        {
            std::println("shutdown failed with error in main loop: {}", WSAGetLastError());
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

struct StreamDecoder
{
    static constexpr usize HEADER = 2;
    static constexpr usize MAX_PAYLOAD = 256;

    std::vector<std::byte> pending;   // bytes received but not yet consumed

    // Returns false if the stream is corrupted (out of sync).
    template <typename OnMessage>
    bool Feed(std::span<const std::byte> data, OnMessage&& onMessage)
    {
        pending.insert(pending.end(), data.begin(), data.end());

        usize offset = 0;
        while (pending.size() - offset >= HEADER)
        {
            const u16 len = static_cast<u16>(
                (std::to_integer<u16>(pending[offset]) << 8) |
                 std::to_integer<u16>(pending[offset + 1]));

            if (len > MAX_PAYLOAD)
                return false;                                // garbage, not a length
            if (pending.size() - offset < HEADER + len)
                break;                                       // message not complete yet

            onMessage(std::span<const std::byte>(pending).subspan(offset + HEADER, len));
            offset += HEADER + len;
        }

        pending.erase(pending.begin(), pending.begin() + offset);
        return true;
    }
};

int main() 
{
    SOCKET client;
    
    MySocket socket(&client);

    StreamDecoder decoder;
    std::array<std::byte, 512> buf;
    usize received = 0;
    
    int result{};
    do 
    {
        result = recv(client, reinterpret_cast<char*>(buf.data()), static_cast<int>(buf.size()), 0);
        if (result > 0) 
        {
            bool ok = decoder.Feed(std::span(buf).first(result), [&](std::span<const std::byte> msg)
            {
                ++received;   // or print: string_view over reinterpret_cast<const char*>(msg.data())
            });

            if (!ok)
            {
                throw;
            }
        }
        else if (result < 0)
        {
            std::println("failed with error in main loop: {}", WSAGetLastError());
            throw;
        }
    } while (result != 0);

    closesocket(client);
    
    std::print("Received: {}", received);
    return 0;
}
