#pragma once

#include <string>
#include "../shared/Network.h"

namespace FleetTelemetry
{
    class ClientSocket
    {
    public:
        ClientSocket() = default;
        ~ClientSocket();

        ClientSocket(const ClientSocket&) = delete;
        ClientSocket& operator=(const ClientSocket&) = delete;

        bool Connect(const std::string& ipAddress, int port, std::string* errorMessage = nullptr);
        bool SendLine(const std::string& line, std::string* errorMessage = nullptr);
        void Disconnect();
        bool IsConnected() const;

    private:
        SocketHandle m_socket = InvalidSocket;
        bool m_connected = false;
    };
}
