#include "ClientApp.h"

#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace
{
    struct LaunchOptions
    {
        int ClientCount = 25;
        int AircraftStart = 1;
        int AircraftEnd = 25;
    };

    int ParseIntOrDefault(const char* text, int defaultValue)
    {
        try
        {
            return std::stoi(text);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    LaunchOptions GetLaunchOptions(int argc, char* argv[])
    {
        LaunchOptions options;

        for (int index = 1; index < argc; ++index)
        {
            const std::string argument = argv[index];

            if (argument == "--client-count" && index + 1 < argc)
            {
                options.ClientCount = ParseIntOrDefault(argv[++index], 3);
            }
            else if (argument == "--aircraft-start" && index + 1 < argc)
            {
                options.AircraftStart = ParseIntOrDefault(argv[++index], 1);
            }
            else if (argument == "--aircraft-end" && index + 1 < argc)
            {
                options.AircraftEnd = ParseIntOrDefault(argv[++index], 3);
            }
        }

        if (options.ClientCount < 1)
        {
            options.ClientCount = 1;
        }

        if (options.AircraftStart < 1)
        {
            options.AircraftStart = 1;
        }

        if (options.AircraftEnd < options.AircraftStart)
        {
            options.AircraftEnd = options.AircraftStart;
        }

        const int availableIds = options.AircraftEnd - options.AircraftStart + 1;
        if (options.ClientCount > availableIds)
        {
            options.ClientCount = availableIds;
        }

        return options;
    }

    std::string BuildAircraftId(int aircraftNumber)
    {
        std::ostringstream stream;
        stream << "AIRCRAFT-";

        if (aircraftNumber < 10)
        {
            stream << "00";
        }
        else if (aircraftNumber < 100)
        {
            stream << "0";
        }

        stream << aircraftNumber;
        return stream.str();
    }
}

int main(int argc, char* argv[])
{
    const LaunchOptions options = GetLaunchOptions(argc, argv);

    std::vector<std::thread> clientThreads;
    clientThreads.reserve(static_cast<std::size_t>(options.ClientCount));

    for (int i = 0; i < options.ClientCount; ++i)
    {
        const int aircraftNumber = options.AircraftStart + i;
        const std::string aircraftId = BuildAircraftId(aircraftNumber);

        clientThreads.emplace_back([argc, argv, aircraftId]()
            {
                std::vector<std::string> args;
                args.reserve(static_cast<std::size_t>(argc) + 2);

                for (int j = 0; j < argc; ++j)
                {
                    args.push_back(argv[j]);
                }

                args.push_back("--aircraft-id");
                args.push_back(aircraftId);

                std::vector<char*> cArgs;
                cArgs.reserve(args.size());
                for (auto& arg : args)
                {
                    cArgs.push_back(arg.data());
                }

                FleetTelemetry::ClientApp app;
                app.Run(static_cast<int>(cArgs.size()), cArgs.data());
            });
    }

    for (auto& thread : clientThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    return 0;
}