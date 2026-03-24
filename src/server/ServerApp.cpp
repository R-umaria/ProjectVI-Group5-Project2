#include "ServerApp.h"
#include "ServerListener.h"
#include "PacketReceiver.h"
#include "AircraftSessionManager.h"
#include "StatisticsStore.h"
#include "../shared/Config.h"
#include "../shared/Logger.h"
#include <vector>

namespace FleetTelemetry
{
    int ServerApp::Run() const
    {
        const auto config = Config::LoadServerConfig("config/server.config.json");
        Logger logger(config.LogFile);
        logger.Info("Server starting");

        ServerListener listener;
        listener.Start(config.ListenPort);

        // Placeholder packets so the scaffold shows end-to-end flow before socket code is implemented.
        const std::vector<std::string> samplePackets =
        {
            "AC001,2026-03-10T10:00:00Z,5200.0",
            "AC001,2026-03-10T10:00:05Z,5197.5",
            "AC002,2026-03-10T10:00:00Z,6100.0"
        };

        PacketReceiver receiver;
        AircraftSessionManager sessions;
        for (const auto& packet : samplePackets)
        {
            TelemetryRecord record;
            if (receiver.ParseIncoming(packet, record))
            {
                sessions.AcceptRecord(record);
                logger.Info("Processed packet for " + record.AircraftId);
            }
        }

        StatisticsStore store;
        store.SaveCsv(config.StatsFile, sessions.GetProcessor().GetCurrentStatistics());

        listener.Stop();
        logger.Info("Server stopped");
        return 0;
    }
}
