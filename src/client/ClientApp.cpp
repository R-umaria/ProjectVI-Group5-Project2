/*!
 * @file ClientApp.cpp
 * @brief Implements the client application responsible for reading telemetry data and sending it to the server.
 */
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
#include <sstream>
#include <vector>
#include <chrono>
#include <cctype>
#include <mutex> //for cout terminal output 

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

std::mutex coutMutex;

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
            bool AircraftIdProvidedByUser = false;

            int ClientCount = 1;
            int AircraftStart = 1;
            int AircraftEnd = 1;
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

        unsigned long GetProcessIdValue()
        {
#ifdef _WIN32
            return static_cast<unsigned long>(::GetCurrentProcessId());
#else
            return static_cast<unsigned long>(::getpid());
#endif
        }

        long long GetSessionTimestampMs()
        {
            return static_cast<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        }

        std::string BuildUniqueAircraftId(const std::string& prefix)
        {
            const std::string normalizedPrefix = MakeSafeFileToken(prefix.empty() ? "AIR" : prefix);
            std::ostringstream stream;
            stream << normalizedPrefix
                << "-PID" << GetProcessIdValue()
                << "-T" << GetSessionTimestampMs();
            return stream.str();
        }

        std::string BuildClientLogPath(const std::string& aircraftId)
        {
            std::ostringstream stream;
            stream << "output/logs/client_"
                << MakeSafeFileToken(aircraftId)
                << "_pid" << GetProcessIdValue()
                << "_session" << GetSessionTimestampMs()
                << ".log";
            return stream.str();
        }

        RuntimeOptions ResolveRuntimeOptions(const ClientConfig& config, int argc, char* argv[])
        {
            RuntimeOptions options;
            options.ServerIp = config.ServerIp;
            options.ServerPort = config.ServerPort;
            options.TelemetryFile = config.TelemetryFile;
            options.SendIntervalMs = config.SendIntervalMs;

            std::string aircraftIdPrefix = config.ClientName;

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
                    options.AircraftIdProvidedByUser = !options.AircraftId.empty();
                }
                else if (argument == "--send-interval-ms")
                {
                    if (index + 1 < argc)
                    {
                        options.SendIntervalMs = std::stoi(argv[++index]);
                    }
                }
                else if (argument == "--client-count")
                {
                    if (index + 1 < argc)
                    {
                        options.ClientCount = std::stoi(argv[++index]);
                    }
                }
                else if (argument == "--aircraft-start")
                {
                    if (index + 1 < argc)
                    {
                        options.AircraftStart = std::stoi(argv[++index]);
                    }
                }
                else if (argument == "--aircraft-end")
                {
                    if (index + 1 < argc)
                    {
                        options.AircraftEnd = std::stoi(argv[++index]);
                    }
                }
            }

            if (options.TelemetryFile.empty())
            {
                options.TelemetryFile = "data/sample/telemetry_1.txt";
            }

            if (!options.AircraftIdProvidedByUser)
            {
                if (aircraftIdPrefix.empty())
                {
                    aircraftIdPrefix = "AIR";
                }

                options.AircraftId = BuildUniqueAircraftId(aircraftIdPrefix);
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

        int totalClients = options.ClientCount;

        if (options.AircraftEnd >= options.AircraftStart)
        {
            totalClients = options.AircraftEnd - options.AircraftStart + 1;
        }

        std::vector<std::thread> threads;

        for (int i = 0; i < totalClients; i++)
        {
            RuntimeOptions clientOptions = options;

            clientOptions.AircraftId = "AIR-" + std::to_string(options.AircraftStart + i);

            clientOptions.TelemetryFile =
                "data/sample/telemetry_" + std::to_string(options.AircraftStart + i) + ".txt";

            threads.push_back(std::thread([clientOptions]()
                {
                    Logger logger(BuildClientLogPath(clientOptions.AircraftId));

                    logger.Info("Client starting");
                    logger.Info("Aircraft ID: " + clientOptions.AircraftId);
                    logger.Info("Telemetry file: " + clientOptions.TelemetryFile);
                    logger.Info("Server endpoint: " + clientOptions.ServerIp + ":" + std::to_string(clientOptions.ServerPort));
                    logger.Info(std::string("Aircraft ID source: ") + (clientOptions.AircraftIdProvidedByUser ? "command line" : "generated at startup"));

                    TelemetryReader reader;
                    if (!reader.Open(clientOptions.TelemetryFile))
                    {
                        logger.Error("Unable to open telemetry file");
                        return;
                    }

                    ClientSocket socket;
                    std::string socketError;

                    logger.Info("Attempting server connection");

                    if (!socket.Connect(clientOptions.ServerIp, clientOptions.ServerPort, &socketError))
                    {
                        logger.Error("Connection failed: " + socketError);
                        reader.Close();
                        return;
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

                        {
                            std::lock_guard<std::mutex> lock(coutMutex);
                            std::cout << "[DEBUG] " << clientOptions.AircraftId << " reading line #" << totalLines << std::endl;
                        }

                        TelemetryRecord record;
                        if (!parser.ParseLine(line, clientOptions.AircraftId, record))
                        {
                            ++skippedLines;
                            continue;
                        }

                        const std::string packet = builder.Build(record);

                        // Check if packet is empty, if packet is empty skip this record
                        if (packet.empty())
                        {
                            logger.Error("Packet validation process failed! " + std::to_string(totalLines) + ". Skipping.");
                            ++skippedLines;
                            continue;
                        }

                        if (!socket.SendLine(packet, &socketError))
                        {
                            logger.Error("Transmission interrupted: " + socketError);
                            socket.Disconnect();
                            reader.Close();
                            return;
                        }

                        ++sentRecords;

                        if ((sentRecords == 1) || (sentRecords % 100 == 0))
                        {
                            logger.Info("Telemetry records transmitted: " + std::to_string(sentRecords));
                        }

                        {
                            std::lock_guard<std::mutex> lock(coutMutex);
                            std::cout << "[SEND] " << clientOptions.AircraftId << " sent packet #" << sentRecords << std::endl;
                        }

                        if (clientOptions.SendIntervalMs > 0)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(clientOptions.SendIntervalMs));
                        }
                    }

                    reader.Close();
                    socket.Disconnect();

                    logger.Info("Client completed flight replay");
                    logger.Info("Total input lines: " + std::to_string(totalLines));
                    logger.Info("Records sent: " + std::to_string(sentRecords));
                    logger.Info("Lines skipped: " + std::to_string(skippedLines));

                    {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "\n[EOF INFO] " << clientOptions.AircraftId << std::endl;
                        std::cout << "Total lines read: " << totalLines << std::endl;
                        std::cout << "Records sent: " << sentRecords << std::endl;
                        std::cout << "Lines skipped: " << skippedLines << std::endl;
                    }

                }));
        }

        for (size_t i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }

        return 0;
    }
}