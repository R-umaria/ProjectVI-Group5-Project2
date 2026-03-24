#include "ClientApp.h"
#include "TelemetryReader.h"
#include "TelemetryParser.h"
#include "ClientSocket.h"
#include "PacketBuilder.h"
#include "../shared/Config.h"
#include "../shared/Logger.h"
#include <thread>

namespace FleetTelemetry
{
    int ClientApp::Run() const
    {
        const auto config = Config::LoadClientConfig("config/client.config.json");
        Logger logger("output/logs/client.log");
        logger.Info("Client starting");

        TelemetryReader reader;
        TelemetryParser parser;
        PacketBuilder builder;
        ClientSocket socket;

        const auto records = reader.ReadAll(config.TelemetryFile);
        logger.Info("Loaded " + std::to_string(records.size()) + " telemetry records");

        socket.Connect(config.ServerIp, config.ServerPort);

        for (const auto& record : records)
        {
            if (!parser.IsValid(record))
            {
                logger.Error("Invalid telemetry record skipped");
                continue;
            }

            socket.SendLine(builder.Build(record));
            std::this_thread::sleep_for(std::chrono::milliseconds(config.SendIntervalMs));
        }

        socket.Disconnect();
        logger.Info("Client finished");
        return 0;
    }
}
