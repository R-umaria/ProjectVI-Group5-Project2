#include "ClientApp.h"
#include "TelemetryReader.h"
#include "TelemetryParser.h"
#include "ClientSocket.h"
#include "PacketBuilder.h"
#include "../shared/Config.h"
#include "../shared/Logger.h"
#include "../shared/Common.h"
#include <thread>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>

namespace FleetTelemetry
{
    namespace
    {
        struct RuntimeOptions
        {
            std::string ServerIp;
            int ServerPort = 0;
            std::string TelemetryFile;
            std::string AircraftId;
            int SendIntervalMs = 0;
        };

        std::string MakeSafeFileToken(std::string value)
        {
            for (char& ch : value)
            {
                if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_'))
                {
                    ch = '_';
                }
            }
            return value;
        }

        std::string GenerateAircraftId(const std::string& prefix)
        {
            std::srand(static_cast<unsigned int>(std::time(nullptr)) ^ static_cast<unsigned int>(std::clock()));
            const int randomValue = std::rand() % 100000;
            std::ostringstream stream;
            stream << (prefix.empty() ? "AIR" : prefix)
                   << "-" << static_cast<long long>(std::time(nullptr))
                   << "-" << randomValue;
            return stream.str();
        }

        RuntimeOptions ResolveRuntimeOptions(const ClientConfig& config, int argc, char* argv[])
        {
            RuntimeOptions options;
            options.ServerIp = config.ServerIp;
            options.ServerPort = config.ServerPort;
            options.TelemetryFile = config.TelemetryFile;
            options.SendIntervalMs = config.SendIntervalMs;
            options.AircraftId = config.ClientName;

            for (int index = 1; index < argc; ++index)
            {
                const std::string argument = argv[index];
                auto readValue = [&](std::string& target)
                {
                    if (index + 1 < argc)
                    {
                        target = argv[++index];
                    }
                };

                if (argument == "--server-ip")
                {
                    readValue(options.ServerIp);
                }
                else if (argument == "--server-port")
                {
                    if (index + 1 < argc)
                    {
                        options.ServerPort = std::stoi(argv[++index]);
                    }
                }
                else if (argument == "--telemetry-file")
                {
                    readValue(options.TelemetryFile);
                }
                else if (argument == "--aircraft-id")
                {
                    readValue(options.AircraftId);
                }
                else if (argument == "--send-interval-ms")
                {
                    if (index + 1 < argc)
                    {
                        options.SendIntervalMs = std::stoi(argv[++index]);
                    }
                }
            }

            if (options.TelemetryFile.empty())
            {
                options.TelemetryFile = "data/sample/telemetry_1.txt";
            }

            if (options.AircraftId.empty())
            {
                options.AircraftId = GenerateAircraftId("AIR");
            }
            else if (options.AircraftId.find("Client-") == 0 || options.AircraftId.find("Client") == 0)
            {
                options.AircraftId = GenerateAircraftId(options.AircraftId);
            }

            if (options.SendIntervalMs < 0)
            {
                options.SendIntervalMs = 0;
            }

            return options;
        }
    }

    int ClientApp::Run(int argc, char* argv[]) const
    {
        const auto config = Config::LoadClientConfig("config/client.config.json");
        const RuntimeOptions options = ResolveRuntimeOptions(config, argc, argv);

        Logger logger("output/logs/client_" + MakeSafeFileToken(options.AircraftId) + ".log");
        logger.Info("Client starting");
        logger.Info("Aircraft ID: " + options.AircraftId);
        logger.Info("Telemetry file: " + options.TelemetryFile);
        logger.Info("Server endpoint: " + options.ServerIp + ":" + std::to_string(options.ServerPort));

        TelemetryReader reader;
        if (!reader.Open(options.TelemetryFile))
        {
            logger.Error("Unable to open telemetry file");
            return 1;
        }

        ClientSocket socket;
        std::string socketError;
        logger.Info("Attempting server connection");
        if (!socket.Connect(options.ServerIp, options.ServerPort, &socketError))
        {
            logger.Error("Connection failed: " + socketError);
            reader.Close();
            return 1;
        }
        logger.Info("Connection established");

        TelemetryParser parser;
        PacketBuilder builder;
        std::string line;
        int totalLines = 0;
        int sentRecords = 0;
        int skippedLines = 0;

        while (reader.ReadNextLine(line))
        {
            ++totalLines;
            TelemetryRecord record;
            if (!parser.ParseLine(line, options.AircraftId, record))
            {
                ++skippedLines;
                continue;
            }

            const std::string packet = builder.Build(record);
            if (!socket.SendLine(packet, &socketError))
            {
                logger.Error("Transmission interrupted: " + socketError);
                socket.Disconnect();
                reader.Close();
                return 1;
            }

            ++sentRecords;
            if (sentRecords == 1 || sentRecords % 100 == 0)
            {
                logger.Info("Telemetry records transmitted: " + std::to_string(sentRecords));
            }

            if (options.SendIntervalMs > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(options.SendIntervalMs));
            }
        }

        reader.Close();
        socket.Disconnect();

        logger.Info("Client completed flight replay");
        logger.Info("Total input lines: " + std::to_string(totalLines));
        logger.Info("Records sent: " + std::to_string(sentRecords));
        logger.Info("Lines skipped: " + std::to_string(skippedLines));
        return 0;
    }
}
