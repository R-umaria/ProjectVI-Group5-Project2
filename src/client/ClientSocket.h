#pragma once

#include <string>

namespace FleetTelemetry
{
    class ClientSocket
    {
    public:
        bool Connect(const std::string& ipAddress, int port);
        bool SendLine(const std::string& line);
        void Disconnect();

    private:
        bool m_connected = false;
    };
}
