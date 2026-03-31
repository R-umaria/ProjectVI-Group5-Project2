#include "ClientApp.h"
#include "TelemetryReader.h"
#include "ClientSocket.h"
#include "PacketBuilder.h"
#include "../shared/Config.h"
#include "../shared/Logger.h"
#include <thread>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

namespace FleetTelemetry
{
    int ClientApp::Run() const
    {
        const auto config = Config::LoadClientConfig("config/client.config.json");
        Logger logger("output/logs/client.log");
        logger.Info("Client starting");

        TelemetryReader reader;
        PacketBuilder builder;
        ClientSocket socket;

        std::vector<std::string> files =
        {
            "data/sample/telemetry_1.txt",
            "data/sample/telemetry_2.txt",
            "data/sample/telemetry_3.txt",
            "data/sample/telemetry_4.txt"
        };

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        int index = std::rand() % static_cast<int>(files.size());

        std::string selectedFile = files[index];
        logger.Info("Selected telemetry file: " + selectedFile);

        const auto records = reader.ReadAll(selectedFile);
        logger.Info("Loaded " + std::to_string(records.size()) + " telemetry records");

        socket.Connect(config.ServerIp, config.ServerPort);

        for (const auto& record : records)
        {
            socket.SendLine(builder.Build(record));
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.SendIntervalMs));
        }

        socket.Disconnect();
        logger.Info("Client finished");
        return 0;
    }
}