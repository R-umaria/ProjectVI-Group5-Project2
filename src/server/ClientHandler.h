#pragma once

#include <string>
#include "PacketReceiver.h"
#include "AircraftSessionManager.h"
#include "../shared/Logger.h"
#include "../shared/Network.h"

namespace FleetTelemetry
{
    class ClientHandler
    {
    public:
        ClientHandler(SocketHandle clientSocket,
            std::string remoteAddress,
            AircraftSessionManager& sessionManager,
            Logger& logger);

        ClientHandler(const ClientHandler&) = delete;
        ClientHandler& operator=(const ClientHandler&) = delete;

        void Run();

    private:
        SocketHandle m_clientSocket = InvalidSocket;
        std::string m_remoteAddress;
        AircraftSessionManager& m_sessionManager;
        Logger& m_logger;
        PacketReceiver m_packetReceiver;
    };
}
