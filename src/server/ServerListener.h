#pragma once

#include <string>
#include "../shared/Network.h"

namespace FleetTelemetry
{
    class ServerListener
    {
    public:
        ServerListener() = default;
        ~ServerListener();

        ServerListener(const ServerListener&) = delete;
        ServerListener& operator=(const ServerListener&) = delete;

        bool Start(const std::string& bindIp, int port, std::string* errorMessage = nullptr);
        bool Accept(SocketHandle& outClientSocket, std::string& outRemoteAddress, std::string* errorMessage = nullptr) const;
        void Stop();
        bool IsRunning() const;
    private:
        SocketHandle m_listenSocket = InvalidSocket;
        bool m_running = false;
    };
}
