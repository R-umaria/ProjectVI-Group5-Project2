/*!
 * @file ServerApp.cpp
 * @brief Runs the server and handles incoming client connections.
 *
 * This module starts the server, listens for clients and creates handlers to process incoming telemetry data.
 */
#include "ServerApp.h"
#include "ServerListener.h"
#include "ClientHandler.h"
#include "AircraftSessionManager.h"
#include "../shared/Config.h"
#include "../shared/Logger.h"
#include "../shared/Common.h"
#include <thread>
#include <vector>
#include <memory>

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

        Logger logger(options.LogFile);
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
        //std::vector<std::thread> workerThreads;

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

            //workerThreads.emplace_back([clientSocket, remoteAddress, &sessions, &logger]() mutable
            std::thread([clientSocket, remoteAddress, &sessions, &logger]() mutable //TO FIX MEMORY ISSUES
            {
                ClientHandler handler(clientSocket, std::move(remoteAddress), sessions, logger);
                handler.Run();
				}).detach(); //added detach and changed from emplace/back to just creating a temporary thread since we don't need to keep track of them. If we wanted to keep track of them, we would need to join them at some point, but since they are detached, they will clean up after themselves when they finish.
            //workerThreads.back().detach();
        }

        return 0;
    }
}
