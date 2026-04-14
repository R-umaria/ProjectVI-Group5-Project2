#pragma once
#include "../shared/Network.h"
#include <string>

namespace FleetTelemetry
{
    class ClientSocket
    {
    public:
        ClientSocket() = default;
        ~ClientSocket();

        bool Connect(const std::string& ipAddress, int port, std::string* errorMessage = nullptr);
        bool SendLine(const std::string& line, std::string* errorMessage = nullptr);

        void Disconnect();
        bool IsConnected() const;

    private:
        SocketHandle clientSocket = InvalidSocket;
        bool isConnected = false;
    };
}