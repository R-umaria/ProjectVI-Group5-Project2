#include "ClientHandler.h"
#include <iostream>

namespace FleetTelemetry
{
    void ClientHandler::HandlePacket(const std::string& packetText) const
    {
        std::cout << "[ClientHandler] Placeholder handling packet: " << packetText << std::endl;
    }
}
