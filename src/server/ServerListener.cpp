/*!
 * @file ServerListener.cpp
 * @brief Listens for incoming client connections.
 *
 * This module sets up the server socket, accepts client connections and manages the listening state.
 */
#include "ServerListener.h"
#include "../shared/Network.h"

namespace FleetTelemetry
{
    ServerListener::~ServerListener()
    {
        Stop();
        Network::Cleanup();
    }

    bool ServerListener::Start(const std::string& bindIp, int port, std::string* errorMessage)
    {
        Stop();

        if (!Network::Initialize(errorMessage))
        {
            return false;
        }

        addrinfo hints{};
        hints.ai_family = AF_INET;
        if (!bindIp.empty() && bindIp.find(":") != std::string::npos)
        {
            hints.ai_family = AF_INET6;
        }
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        const std::string portText = std::to_string(port);
        addrinfo* resultList = nullptr;
        const char* bindAddress = (bindIp == "0.0.0.0" || bindIp.empty()) ? nullptr : bindIp.c_str();
        const int resolveResult = getaddrinfo(bindAddress, portText.c_str(), &hints, &resultList);
        if (resolveResult != 0 || resultList == nullptr)
        {
#ifdef _WIN32
            if (errorMessage != nullptr)
            {
                *errorMessage = "getaddrinfo failed with error " + std::to_string(resolveResult);
            }
#else
            if (errorMessage != nullptr)
            {
                *errorMessage = gai_strerror(resolveResult);
            }
#endif
            Network::Cleanup();
            return false;
        }

        bool started = false;
        std::string lastStartError = "Unable to bind/listen on requested endpoint";
        for (addrinfo* current = resultList; current != nullptr; current = current->ai_next)
        {
            m_listenSocket = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
            if (m_listenSocket == InvalidSocket)
            {
                lastStartError = Network::GetLastErrorString();
                continue;
            }

            Network::SetReuseAddress(m_listenSocket, nullptr);
#ifdef _WIN32
            if (current->ai_family == AF_INET6)
            {
                DWORD dualStackOff = 0;
                setsockopt(
                    m_listenSocket,
                    IPPROTO_IPV6,
                    IPV6_V6ONLY,
                    reinterpret_cast<const char*>(&dualStackOff),
                    static_cast<int>(sizeof(dualStackOff)));
            }
#endif
            if (bind(m_listenSocket, current->ai_addr, static_cast<int>(current->ai_addrlen)) != 0)
            {
                lastStartError = Network::GetLastErrorString();
                Network::CloseSocket(m_listenSocket);
                m_listenSocket = InvalidSocket;
                continue;
            }

            if (listen(m_listenSocket, SOMAXCONN) != 0)
            {
                lastStartError = Network::GetLastErrorString();
                Network::CloseSocket(m_listenSocket);
                m_listenSocket = InvalidSocket;
                continue;
            }

            started = true;
            break;
        }

        freeaddrinfo(resultList);

        if (!started)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = lastStartError;
            }
            Network::Cleanup();
            return false;
        }

        m_running = true;
        return true;
    }

    bool ServerListener::Accept(SocketHandle& outClientSocket, std::string& outRemoteAddress, std::string* errorMessage) const
    {
        outClientSocket = InvalidSocket;
        outRemoteAddress.clear();

        sockaddr_storage clientAddress{};
        socklen_t addressLength = static_cast<socklen_t>(sizeof(clientAddress));
        SocketHandle clientSocket = accept(m_listenSocket, reinterpret_cast<sockaddr*>(&clientAddress), &addressLength);
        if (clientSocket == InvalidSocket)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = Network::GetLastErrorString();
            }
            return false;
        }

        char hostBuffer[NI_MAXHOST] = {};
        if (getnameinfo(reinterpret_cast<sockaddr*>(&clientAddress),
            addressLength,
            hostBuffer,
            sizeof(hostBuffer),
            nullptr,
            0,
            NI_NUMERICHOST) == 0)
        {
            outRemoteAddress = hostBuffer;
        }
        else
        {
            outRemoteAddress = "unknown";
        }

        outClientSocket = clientSocket;
        return true;
    }

    void ServerListener::Stop()
    {
        if (m_listenSocket != InvalidSocket)
        {
            Network::ShutdownSocket(m_listenSocket);
            Network::CloseSocket(m_listenSocket);
            m_listenSocket = InvalidSocket;
        }
        m_running = false;
    }

    bool ServerListener::IsRunning() const
    {
        return m_running;
    }
}
