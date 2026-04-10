#include "ServerApp.h"
#include "ServerListener.h"
#include "ClientHandler.h"
#include "AircraftSessionManager.h"
#include "../shared/Config.h"
#include "../shared/Logger.h"
#include "../shared/Common.h"
#include <thread>

namespace FleetTelemetry
{
    namespace
    {
        struct RuntimeOptions
        {
            std::string BindIp;
            int ListenPort = 0;
            std::string StatsFile;
            std::string LogFile;
        };

        RuntimeOptions ResolveRuntimeOptions(const ServerConfig& config, int argc, char* argv[])
        {
            RuntimeOptions options;
            options.BindIp = config.BindIp;
            options.ListenPort = config.ListenPort;
            options.StatsFile = config.StatsFile;
            options.LogFile = config.LogFile;

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

                if (argument == "--bind-ip")
                {
                    readValue(options.BindIp);
                }
                else if (argument == "--listen-port")
                {
                    if (index + 1 < argc)
                    {
                        options.ListenPort = std::stoi(argv[++index]);
                    }
                }
                else if (argument == "--stats-file")
                {
                    readValue(options.StatsFile);
                }
                else if (argument == "--log-file")
                {
                    readValue(options.LogFile);
                }
            }

            return options;
        }
    }

    int ServerApp::Run(int argc, char* argv[]) const
    {
        const auto config = Config::LoadServerConfig("config/server.config.json");
        const RuntimeOptions options = ResolveRuntimeOptions(config, argc, argv);

        Logger logger(options.LogFile, true);
        logger.Info("Server starting");

        ServerListener listener;
        std::string listenerError;
        if (!listener.Start(options.BindIp, options.ListenPort, &listenerError))
        {
            logger.Error("Failed to start listener: " + listenerError);
            return 1;
        }

        logger.Info("Server listening on " + options.BindIp + ":" + std::to_string(options.ListenPort));
        logger.Info("Persisting completed flight statistics to " + options.StatsFile);

        AircraftSessionManager sessions(options.StatsFile, logger);

        while (true)
        {
            SocketHandle clientSocket = InvalidSocket;
            std::string remoteAddress;
            std::string acceptError;

            if (!listener.Accept(clientSocket, remoteAddress, &acceptError))
            {
                logger.Error("Accept failed: " + acceptError);
                continue;
            }

            std::thread([clientSocket, remoteAddress, &sessions, &logger]() mutable
            {
                ClientHandler handler(clientSocket, std::move(remoteAddress), sessions, logger);
                handler.Run();
            }).detach();
        }

        return 0;
    }
}
