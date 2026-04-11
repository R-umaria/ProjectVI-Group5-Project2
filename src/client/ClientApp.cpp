#include "ClientApp.h"
#include "TelemetryReader.h"
#include "TelemetryParser.h"
#include "ClientSocket.h"
#include "PacketBuilder.h"
#include "../shared/Config.h"
#include "../shared/Logger.h"
#include "../shared/Common.h"
#include "../shared/PathUtils.h"

#include <algorithm>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <random>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

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
            bool TelemetryFileProvidedByUser = false;
            bool VerboseRecords = false;
            bool UseRandomTelemetryFile = false;
        };

        std::mutex g_consoleMutex;

        int ParseIntOrDefault(const char* text, int fallback)
        {
            try
            {
                return std::stoi(text);
            }
            catch (...)
            {
                return fallback;
            }
        }

        unsigned long GetCurrentProcessIdValue()
        {
#ifdef _WIN32
            return static_cast<unsigned long>(::GetCurrentProcessId());
#else
            return static_cast<unsigned long>(::getpid());
#endif
        }

        class CounterLock
        {
        public:
            explicit CounterLock(std::filesystem::path lockPath) :
                m_lockPath(std::move(lockPath))
            {
            }

            ~CounterLock()
            {
                Release();
            }

            CounterLock(const CounterLock&) = delete;
            CounterLock& operator=(const CounterLock&) = delete;

            bool Acquire()
            {
                namespace fs = std::filesystem;
                std::error_code ec;
                fs::create_directories(m_lockPath.parent_path(), ec);

                for (int attempt = 0; attempt < 200; ++attempt)
                {
                    if (fs::create_directory(m_lockPath, ec))
                    {
                        m_acquired = true;
                        return true;
                    }

                    ec.clear();
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }

                return false;
            }

        private:
            void Release()
            {
                if (!m_acquired)
                {
                    return;
                }

                std::error_code ec;
                std::filesystem::remove_all(m_lockPath, ec);
                m_acquired = false;
            }

            std::filesystem::path m_lockPath;
            bool m_acquired = false;
        };

        int ReadIntFromFile(const std::filesystem::path& filePath, int fallback)
        {
            std::ifstream input(filePath);
            if (!input)
            {
                return fallback;
            }

            int value = fallback;
            input >> value;
            return input ? value : fallback;
        }

        bool WriteIntToFile(const std::filesystem::path& filePath, int value)
        {
            std::ofstream output(filePath, std::ios::trunc);
            if (!output)
            {
                return false;
            }

            output << value;
            output.flush();
            return static_cast<bool>(output);
        }

        int ReadAircraftIdOffset()
        {
            const char* offsetText = std::getenv("FLEET_AIRCRAFT_ID_OFFSET");
            if (offsetText == nullptr || *offsetText == '\0')
            {
                return 0;
            }

            return std::max(0, ParseIntOrDefault(offsetText, 0));
        }

        std::string ResolveAircraftPrefix(const std::string& configuredPrefix)
        {
            const char* environmentPrefix = std::getenv("FLEET_AIRCRAFT_PREFIX");
            if (environmentPrefix != nullptr && *environmentPrefix != '\0')
            {
                return environmentPrefix;
            }

            return configuredPrefix.empty() ? std::string("Aircraft") : configuredPrefix;
        }

        std::string FormatAircraftId(const std::string& prefix, int aircraftNumber)
        {
            std::ostringstream stream;
            stream << prefix << '_' << std::setw(3) << std::setfill('0') << aircraftNumber;
            return stream.str();
        }

        std::string BuildGeneratedAircraftId(const std::string& configuredPrefix)
        {
            const auto counterFile = FleetTelemetry::PathUtils::ResolveWritablePath("output/state/aircraft_id_counter.txt");
            const auto lockPath = FleetTelemetry::PathUtils::ResolveWritablePath("output/state/aircraft_id_counter.lock");
            const std::string prefix = ResolveAircraftPrefix(configuredPrefix);
            const int offset = ReadAircraftIdOffset();

            CounterLock lock(lockPath);
            if (lock.Acquire())
            {
                const int nextLocalSequence = ReadIntFromFile(counterFile, 0) + 1;
                (void)WriteIntToFile(counterFile, nextLocalSequence);
                return FormatAircraftId(prefix, offset + nextLocalSequence);
            }

            return FormatAircraftId(prefix, offset + static_cast<int>(GetCurrentProcessIdValue() % 1000UL));
        }

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
            const auto sampleDirectory = FleetTelemetry::PathUtils::ResolveExistingPath("data/sample");
            std::vector<std::string> files = FindTelemetryFiles(sampleDirectory.string());
            if (files.empty())
            {
                return FleetTelemetry::PathUtils::ResolveExistingPath("data/sample/telemetry_1.txt").string();
            }

            std::sort(files.begin(), files.end());

            std::random_device randomDevice;
            const auto nowTicks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::seed_seq seed
            {
                static_cast<unsigned int>(randomDevice()),
                static_cast<unsigned int>(GetCurrentProcessIdValue()),
                static_cast<unsigned int>(nowTicks & 0xffffffff),
                static_cast<unsigned int>((nowTicks >> 32) & 0xffffffff)
            };
            std::mt19937 generator(seed);
            std::uniform_int_distribution<std::size_t> distribution(0, files.size() - 1);
            return files[distribution(generator)];
        }

        RuntimeOptions ResolveRuntimeOptions(const ClientConfig& config, int argc, char* argv[])
        {
            RuntimeOptions options;
            options.ServerIp = config.ServerIp;
            options.ServerPort = config.ServerPort;
            options.TelemetryFile = FleetTelemetry::PathUtils::ResolveExistingPath(config.TelemetryFile).string();
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
                else if (argument == "--server-port" && index + 1 < argc)
                {
                    options.ServerPort = ParseIntOrDefault(argv[++index], options.ServerPort);
                }
                else if (argument == "--telemetry-file")
                {
                    readValue(options.TelemetryFile);
                    options.TelemetryFile = FleetTelemetry::PathUtils::ResolveExistingPath(options.TelemetryFile).string();
                    options.TelemetryFileProvidedByUser = !options.TelemetryFile.empty();
                }
                else if (argument == "--aircraft-id")
                {
                    readValue(options.AircraftId);
                    options.AircraftIdProvidedByUser = !options.AircraftId.empty();
                }
                else if (argument == "--send-interval-ms" && index + 1 < argc)
                {
                    options.SendIntervalMs = ParseIntOrDefault(argv[++index], options.SendIntervalMs);
                }
                else if (argument == "--verbose-records")
                {
                    options.VerboseRecords = true;
                }
                else if (argument == "--random-telemetry-file")
                {
                    options.UseRandomTelemetryFile = true;
                }
            }

            if (options.UseRandomTelemetryFile ||
                !options.TelemetryFileProvidedByUser ||
                options.TelemetryFile.empty() ||
                !std::filesystem::exists(options.TelemetryFile))
            {
                options.TelemetryFile = SelectRandomTelemetryFile();
            }

            if (!options.AircraftIdProvidedByUser)
            {
                options.AircraftId = BuildGeneratedAircraftId(config.ClientName);
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
            std::cout << "Server        : " << options.ServerIp << ':' << options.ServerPort << "\n";
            std::cout << "==================================================\n";
        }

        void PrintParsedRecord(const TelemetryRecord& record)
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << '[' << record.AircraftId << "] Timestamp: " << record.Timestamp
                      << " | Fuel Quantity: " << record.FuelQuantity << "\n";
        }

        void PrintClientFooter(const std::string& aircraftId,
            int totalLines,
            int sentRecords,
            int skippedLines,
            bool completedTransmission)
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "--------------------------------------------------\n";
            std::cout << "Client Finished: " << aircraftId << "\n";
            std::cout << "Total input lines: " << totalLines << "\n";
            std::cout << "Records sent     : " << sentRecords << "\n";
            std::cout << "Lines skipped    : " << skippedLines << "\n";
            if (completedTransmission)
            {
                std::cout << "EOF reached. File closed. Client terminated.\n";
            }
            else
            {
                std::cout << "Transmission terminated safely before EOF.\n";
            }
            std::cout << "--------------------------------------------------\n";
        }
    }

    int ClientApp::Run(int argc, char* argv[]) const
    {
        const auto configPath = FleetTelemetry::PathUtils::ResolveExistingPath("config/client.config.json");
        const auto config = Config::LoadClientConfig(configPath.string());
        const RuntimeOptions options = ResolveRuntimeOptions(config, argc, argv);

        const auto logPath = FleetTelemetry::PathUtils::ResolveWritablePath("output/logs/" + options.AircraftId + ".log");
        Logger logger(logPath.string(), false);
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
        if (!socket.Connect(options.ServerIp, options.ServerPort, &socketError))
        {
            logger.Error("Connection failed: " + socketError);
            std::lock_guard<std::mutex> lock(g_consoleMutex);
            std::cout << "ERROR: Unable to connect to server for "
                      << options.AircraftId << ": " << socketError << "\n";
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
        bool transmissionComplete = true;

        while (reader.ReadNextLine(line))
        {
            ++totalLines;

            TelemetryRecord record;
            if (!parser.ParseLine(line, options.AircraftId, record))
            {
                ++skippedLines;
                continue;
            }

            if (options.VerboseRecords)
            {
                PrintParsedRecord(record);
            }

            const std::string packet = builder.Build(record);
            if (!socket.SendLine(packet, &socketError))
            {
                logger.Error("Transmission interrupted: " + socketError);
                std::lock_guard<std::mutex> lock(g_consoleMutex);
                std::cout << "WARNING: Send failed for "
                          << options.AircraftId
                          << ". Client is terminating safely.\n";
                transmissionComplete = false;
                break;
            }

            ++sentRecords;

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

        PrintClientFooter(options.AircraftId, totalLines, sentRecords, skippedLines, transmissionComplete);
        return transmissionComplete ? 0 : 1;
    }
}
