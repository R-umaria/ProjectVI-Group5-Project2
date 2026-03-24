#include "ClientSocket.h"
#include <iostream>

namespace FleetTelemetry
{
    bool ClientSocket::Connect(const std::string& ipAddress, int port)
    {
        std::cout << "[ClientSocket] Placeholder connect to " << ipAddress << ":" << port << std::endl;
        m_connected = true;
        return true;
    }

    bool ClientSocket::SendLine(const std::string& line)
    {
        if (!m_connected)
        {
            return false;
        }

        std::cout << "[ClientSocket] Placeholder send: " << line << std::endl;
        return true;
    }

    void ClientSocket::Disconnect()
    {
        if (m_connected)
        {
            std::cout << "[ClientSocket] Placeholder disconnect" << std::endl;
        }
        m_connected = false;
    }
}
