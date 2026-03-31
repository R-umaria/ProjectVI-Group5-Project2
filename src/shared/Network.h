#pragma once

#include <string>
#include <mutex>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace FleetTelemetry
{
#ifdef _WIN32
    using SocketHandle = SOCKET;
    constexpr SocketHandle InvalidSocket = INVALID_SOCKET;
#else
    using SocketHandle = int;
    constexpr SocketHandle InvalidSocket = -1;
#endif

    namespace Network
    {
        inline std::mutex g_networkMutex;
        inline int g_networkUserCount = 0;

        inline std::string GetLastErrorString()
        {
#ifdef _WIN32
            return "WSA error " + std::to_string(WSAGetLastError());
#else
            return std::strerror(errno);
#endif
        }

        inline bool Initialize(std::string* errorMessage = nullptr)
        {
#ifdef _WIN32
            std::lock_guard<std::mutex> lock(g_networkMutex);
            if (g_networkUserCount == 0)
            {
                WSADATA wsaData{};
                const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
                if (result != 0)
                {
                    if (errorMessage != nullptr)
                    {
                        *errorMessage = "WSAStartup failed with error " + std::to_string(result);
                    }
                    return false;
                }
            }
            ++g_networkUserCount;
#else
            (void)errorMessage;
#endif
            return true;
        }

        inline void Cleanup()
        {
#ifdef _WIN32
            std::lock_guard<std::mutex> lock(g_networkMutex);
            if (g_networkUserCount > 0)
            {
                --g_networkUserCount;
                if (g_networkUserCount == 0)
                {
                    WSACleanup();
                }
            }
#endif
        }

        inline void CloseSocket(SocketHandle socketHandle)
        {
            if (socketHandle == InvalidSocket)
            {
                return;
            }
#ifdef _WIN32
            closesocket(socketHandle);
#else
            close(socketHandle);
#endif
        }

        inline void ShutdownSocket(SocketHandle socketHandle)
        {
            if (socketHandle == InvalidSocket)
            {
                return;
            }
#ifdef _WIN32
            shutdown(socketHandle, SD_BOTH);
#else
            shutdown(socketHandle, SHUT_RDWR);
#endif
        }

        inline bool SendAll(SocketHandle socketHandle, const std::string& data, std::string* errorMessage = nullptr)
        {
            size_t totalBytesSent = 0;
            while (totalBytesSent < data.size())
            {
                const int bytesSent = send(
                    socketHandle,
                    data.data() + totalBytesSent,
                    static_cast<int>(data.size() - totalBytesSent),
                    0);

                if (bytesSent <= 0)
                {
                    if (errorMessage != nullptr)
                    {
                        *errorMessage = GetLastErrorString();
                    }
                    return false;
                }

                totalBytesSent += static_cast<size_t>(bytesSent);
            }

            return true;
        }

        inline bool SetReuseAddress(SocketHandle socketHandle, std::string* errorMessage = nullptr)
        {
            const int optValue = 1;
            const int result = setsockopt(
                socketHandle,
                SOL_SOCKET,
                SO_REUSEADDR,
#ifdef _WIN32
                reinterpret_cast<const char*>(&optValue),
#else
                &optValue,
#endif
                static_cast<socklen_t>(sizeof(optValue)));

            if (result != 0)
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = GetLastErrorString();
                }
                return false;
            }

            return true;
        }
    }
}
