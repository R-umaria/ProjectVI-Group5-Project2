#pragma once

#include <string>

namespace FleetTelemetry
{
    class ClientHandler
    {
    public:
        void HandlePacket(const std::string& packetText) const;
    };
}
