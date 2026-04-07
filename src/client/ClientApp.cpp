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
#include <random>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <mutex>

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
        };

        std::atomic<int> g_aircraftSequence{ 0 };
        std::mutex g_consoleMutex;

        std::string BuildReadableAircraftId()
        {
            const int id = ++g_aircraftSequence;

            std::ostringstream stream;
            stream << "AIRCRAFT-" << std::setw(3) << std::setfill('0') << id;
            return stream.str();
        }

#include <filesystem>
#include <vector>
#include <string>

        std::vector<std::string> FindTelemetryFiles(const std::string& directoryPath)
        {
            std::vector<std::string> files;
            namespace fs = std::filesystem;

            try
            {
                if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath))
                {
                    return files;
                }

                for (const auto& entry : fs::directory_iterator(directoryPath))
                {
                    if (!entry.is_regular_file())
                    {
                        continue;
                    }

                    const std::string extension = entry.path().extension().string();
                    if (extension == ".txt" || extension == ".csv")
                    {
                        files.push_back(entry.path().string());
                    }
                }
            }
            catch (...)
            {
                files.clear();
            }

            return files;
        }

        std::string SelectRandomTelemetryFile()
        {
            std::vector<std::string> files = FindTelemetryFiles("data/sample");

            if (files.empty())
            {
                return "data/sample/telemetry_1.txt";
            }

            static thread_local std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<std::size_t> dist(0, files.size() - 1);
            std::cout << "Found " << files.size() << " telemetry files in data/sample\n";
            
            return files[dist(gen)];
        }

        RuntimeOptions ResolveRuntimeOptions(const ClientConfig& config, int argc, char* argv[])
        {
            RuntimeOptions options;
            options.ServerIp = config.ServerIp;
            options.ServerPort = config.ServerPort;
            options.TelemetryFile = config.TelemetryFile;
            options.SendIntervalMs = config.SendIntervalMs;

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
            }

            options.TelemetryFile = SelectRandomTelemetryFile();

            if (!options.AircraftIdProvidedByUser)
            {
                options.AircraftId = BuildReadableAircraftId();
            }

            if (options.SendIntervalMs < 0)
            {
                options.SendIntervalMs = 0;
            }

            return options;
        }

        void PrintClientHeader(const RuntimeOptions& options)
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "\n==================================================\n";
            std::cout << "Client Started\n";
            std::cout << "Airplane ID   : " << options.AircraftId << "\n";
            std::cout << "Telemetry File: " << options.TelemetryFile << "\n";
            std::cout << "==================================================\n";
        }

        void PrintParsedRecord(const TelemetryRecord& record)
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "[" << record.AircraftId << "] "
                << "Timestamp: " << record.Timestamp
                << " | Fuel Quantity: " << record.FuelQuantity
                << "\n";
        }

        void PrintClientFooter(const std::string& aircraftId,
            int totalLines,
            int sentRecords,
            int skippedLines)
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "--------------------------------------------------\n";
            std::cout << "Client Finished: " << aircraftId << "\n";
            std::cout << "Total input lines: " << totalLines << "\n";
            std::cout << "Records parsed   : " << sentRecords << "\n";
            std::cout << "Lines skipped    : " << skippedLines << "\n";
            std::cout << "EOF reached. File closed. Client terminated.\n";
            std::cout << "--------------------------------------------------\n";
        }
    }

    int ClientApp::Run(int argc, char* argv[]) const
    {
        const auto config = Config::LoadClientConfig("config/client.config.json");
        const RuntimeOptions options = ResolveRuntimeOptions(config, argc, argv);

        Logger logger("output/logs/" + options.AircraftId + ".log");
        logger.Info("Client starting");
        logger.Info("Aircraft ID: " + options.AircraftId);
        logger.Info("Telemetry file: " + options.TelemetryFile);

        PrintClientHeader(options);

        TelemetryReader reader;
        if (!reader.Open(options.TelemetryFile))
        {
            logger.Error("Unable to open telemetry file");

            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "ERROR: Unable to open telemetry file: "
                << options.TelemetryFile << "\n";
            return 1;
        }

        ClientSocket socket;
        std::string socketError;
        bool serverConnected = false;

        if (socket.Connect(options.ServerIp, options.ServerPort, &socketError))
        {
            serverConnected = true;
            logger.Info("Connection established");
        }
        else
        {
            logger.Error("Connection failed: " + socketError);
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "NOTE: Server connection failed for "
                << options.AircraftId
                << ". Continuing in display-only mode.\n";
        }

        TelemetryParser parser;
        PacketBuilder builder;
        std::string line;
        int totalLines = 0;
        int parsedRecords = 0;
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

            ++parsedRecords;
            PrintParsedRecord(record);

            if (serverConnected)
            {
                const std::string packet = builder.Build(record);
                if (!socket.SendLine(packet, &socketError))
                {
                    logger.Error("Transmission interrupted: " + socketError);
                    std::lock_guard<std::mutex> lock(g_consoleMutex);
                    std::cout << "WARNING: Send failed for "
                        << options.AircraftId
                        << ". Continuing local display only.\n";
                    socket.Disconnect();
                    serverConnected = false;
                }
            }

            if (options.SendIntervalMs > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(options.SendIntervalMs));
            }
        }

        reader.Close();

        if (serverConnected)
        {
            socket.Disconnect();
        }

        logger.Info("Client completed flight replay");
        logger.Info("Total input lines: " + std::to_string(totalLines));
        logger.Info("Records parsed: " + std::to_string(parsedRecords));
        logger.Info("Lines skipped: " + std::to_string(skippedLines));

        PrintClientFooter(options.AircraftId, totalLines, parsedRecords, skippedLines);
        return 0;
    }
}