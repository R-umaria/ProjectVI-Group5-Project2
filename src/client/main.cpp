#include "ClientApp.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
    struct LaunchOptions
    {
        bool LauncherMode = false;
        bool ClientCountProvided = false;
        bool AircraftStartProvided = false;
        bool AircraftEndProvided = false;

        int ClientCount = 1;
        int AircraftStart = 1;
        int AircraftEnd = 1;
        std::string AircraftPrefix = "AIRCRAFT";
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

    bool IsLauncherArgument(const std::string& argument)
    {
        return argument == "--client-count"
            || argument == "--aircraft-start"
            || argument == "--aircraft-end"
            || argument == "--aircraft-prefix";
    }

    LaunchOptions GetLaunchOptions(int argc, char* argv[])
    {
        LaunchOptions options;

        for (int index = 1; index < argc; ++index)
        {
            const std::string argument = argv[index];

            if (argument == "--client-count" && index + 1 < argc)
            {
                options.ClientCount = ParseIntOrDefault(argv[++index], 1);
                options.ClientCountProvided = true;
                options.LauncherMode = true;
            }
            else if (argument == "--aircraft-start" && index + 1 < argc)
            {
                options.AircraftStart = ParseIntOrDefault(argv[++index], 1);
                options.AircraftStartProvided = true;
                options.LauncherMode = true;
            }
            else if (argument == "--aircraft-end" && index + 1 < argc)
            {
                options.AircraftEnd = ParseIntOrDefault(argv[++index], options.AircraftEnd);
                options.AircraftEndProvided = true;
                options.LauncherMode = true;
            }
            else if (argument == "--aircraft-prefix" && index + 1 < argc)
            {
                options.AircraftPrefix = argv[++index];
                options.LauncherMode = true;
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

        if (!options.AircraftEndProvided)
        {
            options.AircraftEnd = options.AircraftStart + options.ClientCount - 1;
        }

        if (options.AircraftEnd < options.AircraftStart)
        {
            options.AircraftEnd = options.AircraftStart;
        }

        const int availableIds = options.AircraftEnd - options.AircraftStart + 1;

        if (!options.ClientCountProvided)
        {
            options.ClientCount = availableIds;
        }
        else if (options.ClientCount > availableIds)
        {
            options.ClientCount = availableIds;
        }

        if (options.AircraftPrefix.empty())
        {
            options.AircraftPrefix = "AIRCRAFT";
        }

        return options;
    }

    std::string BuildAircraftId(const std::string& prefix, int aircraftNumber)
    {
        std::ostringstream stream;
        stream << prefix << '-';

        if (aircraftNumber < 10)
        {
            stream << "00";
        }
        else if (aircraftNumber < 100)
        {
            stream << '0';
        }

        stream << aircraftNumber;
        return stream.str();
    }

    std::vector<std::string> BuildBaseChildArguments(int argc, char* argv[])
    {
        std::vector<std::string> args;
        args.reserve(static_cast<std::size_t>(argc));

        for (int index = 1; index < argc; ++index)
        {
            const std::string argument = argv[index];
            if (argument == "--aircraft-id")
            {
                ++index;
                continue;
            }

            if (IsLauncherArgument(argument))
            {
                ++index;
                continue;
            }

            args.push_back(argument);
        }

        return args;
    }

#ifdef _WIN32
    std::string QuoteIfNeeded(const std::string& value)
    {
        if (value.find_first_of(" \t\"") == std::string::npos)
        {
            return value;
        }

        std::string escaped = "\"";
        for (char ch : value)
        {
            if (ch == '\"')
            {
                escaped += "\\\"";
            }
            else
            {
                escaped += ch;
            }
        }
        escaped += '\"';
        return escaped;
    }

    bool SpawnChildProcess(const std::string& executablePath, const std::vector<std::string>& arguments)
    {
        std::ostringstream commandLine;
        commandLine << QuoteIfNeeded(executablePath);
        for (const auto& argument : arguments)
        {
            commandLine << ' ' << QuoteIfNeeded(argument);
        }

        std::string commandLineBuffer = commandLine.str();

        STARTUPINFOA startupInfo{};
        startupInfo.cb = sizeof(startupInfo);
        PROCESS_INFORMATION processInfo{};

        const BOOL created = CreateProcessA(
            nullptr,
            commandLineBuffer.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE | CREATE_MINIMIZED,
            nullptr,
            nullptr,
            &startupInfo,
            &processInfo);

        if (!created)
        {
            return false;
        }

        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return true;
    }
#else
    bool SpawnChildProcess(const std::string& executablePath, const std::vector<std::string>& arguments)
    {
        std::ostringstream command;
        command << '"' << executablePath << '"';
        for (const auto& argument : arguments)
        {
            command << ' ' << '"' << argument << '"';
        }
        command << " &";
        return std::system(command.str().c_str()) == 0;
    }
#endif

    int RunLauncherMode(int argc, char* argv[])
    {
        const LaunchOptions options = GetLaunchOptions(argc, argv);
        const std::vector<std::string> baseArguments = BuildBaseChildArguments(argc, argv);
        const std::string executablePath = argv[0];

        int launchedCount = 0;
        for (int index = 0; index < options.ClientCount; ++index)
        {
            const int aircraftNumber = options.AircraftStart + index;
            std::vector<std::string> childArguments = baseArguments;
            childArguments.push_back("--aircraft-id");
            childArguments.push_back(BuildAircraftId(options.AircraftPrefix, aircraftNumber));

            if (SpawnChildProcess(executablePath, childArguments))
            {
                ++launchedCount;
            }
            else
            {
                std::cerr << "Failed to launch client for aircraft number " << aircraftNumber << "\n";
            }
        }

        std::cout << "Launched " << launchedCount
                  << " client process(es) covering IDs "
                  << BuildAircraftId(options.AircraftPrefix, options.AircraftStart)
                  << " to "
                  << BuildAircraftId(options.AircraftPrefix, options.AircraftStart + std::max(launchedCount - 1, 0))
                  << ".\n";

        return launchedCount > 0 ? 0 : 1;
    }
}

int main(int argc, char* argv[])
{
    const LaunchOptions options = GetLaunchOptions(argc, argv);

    if (options.LauncherMode)
    {
        return RunLauncherMode(argc, argv);
    }

    FleetTelemetry::ClientApp app;
    return app.Run(argc, argv);
}
