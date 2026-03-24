#include "ServerListener.h"
#include <iostream>

namespace FleetTelemetry
{
    bool ServerListener::Start(int port)
    {
        std::cout << "[ServerListener] Placeholder start on port " << port << std::endl;
        m_running = true;
        return true;
    }

    void ServerListener::Stop()
    {
        if (m_running)
        {
            std::cout << "[ServerListener] Placeholder stop" << std::endl;
        }
        m_running = false;
    }
}
