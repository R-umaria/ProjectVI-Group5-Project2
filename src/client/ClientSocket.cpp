/*!
 * @file ClientSocket.cpp
 * @brief Handles TCP client socket operations such as connecting, sending data and disconnecting from the server.
 */
#include "ClientSocket.h"
#include "../shared/Network.h"
#include <string>

namespace FleetTelemetry
{
    ClientSocket::~ClientSocket()
    {
        Disconnect();
        Network::Cleanup();
    }

    bool ClientSocket::Connect(const std::string& ipAddress, int port, std::string* errorMessage)
    {
        Disconnect();

        if (!Network::Initialize(errorMessage))
        {
            return false;
        }

        addrinfo hints{};
        hints.ai_family = AF_INET;
        if (ipAddress.find(":") != std::string::npos)
        {
            hints.ai_family = AF_INET6;
        }
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* resultList = nullptr;
        const std::string portText = std::to_string(port);
        const int resolveResult = getaddrinfo(ipAddress.c_str(), portText.c_str(), &hints, &resultList);
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

        bool connected = false;
        std::string lastConnectError = "Unable to connect to server";
        for (addrinfo* current = resultList; current != nullptr; current = current->ai_next)
        {
            m_socket = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
            if (m_socket == InvalidSocket)
            {
                lastConnectError = Network::GetLastErrorString();
                continue;
            }

            if (connect(m_socket, current->ai_addr, static_cast<int>(current->ai_addrlen)) == 0)
            {
                connected = true;
                break;
            }

            lastConnectError = Network::GetLastErrorString();
            Network::CloseSocket(m_socket);
            m_socket = InvalidSocket;
        }

        freeaddrinfo(resultList);

        if (!connected)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = lastConnectError;
            }
            Network::Cleanup();
            return false;
        }

        m_connected = true;
        return true;
    }

    bool ClientSocket::SendLine(const std::string& line, std::string* errorMessage)
    {
        if (!m_connected)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = "Socket is not connected";
            }
            return false;
        }

        return Network::SendAll(m_socket, line + "\n", errorMessage);
    }

    void ClientSocket::Disconnect()
    {
        if (m_socket != InvalidSocket)
        {
            Network::ShutdownSocket(m_socket);
            Network::CloseSocket(m_socket);
            m_socket = InvalidSocket;
        }
        m_connected = false;
    }

    bool ClientSocket::IsConnected() const
    {
        return m_connected;
    }
}
