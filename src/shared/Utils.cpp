#include "Config.h"

namespace FleetTelemetry
{
    std::string Config::ReadFile(const std::string& path)
    {
        std::ifstream input(path);
        if (!input)
        {
            return "";
        }

        std::stringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    std::string Config::ReadJsonString(const std::string& json, const std::string& key, const std::string& fallback)
    {
        const auto token = "\"" + key + "\"";
        const auto keyPos = json.find(token);
        if (keyPos == std::string::npos)
        {
            return fallback;
        }

        const auto colonPos = json.find(':', keyPos);
        const auto firstQuote = json.find('"', colonPos + 1);
        const auto secondQuote = json.find('"', firstQuote + 1);
        if (colonPos == std::string::npos || firstQuote == std::string::npos || secondQuote == std::string::npos)
        {
            return fallback;
        }

        return json.substr(firstQuote + 1, secondQuote - firstQuote - 1);
    }

    int Config::ReadJsonInt(const std::string& json, const std::string& key, int fallback)
    {
        const auto token = "\"" + key + "\"";
        const auto keyPos = json.find(token);
        if (keyPos == std::string::npos)
        {
            return fallback;
        }

        const auto colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos)
        {
            return fallback;
        }

        auto endPos = json.find_first_of(",}\n\r", colonPos + 1);
        if (endPos == std::string::npos)
        {
            endPos = json.size();
        }

        try
        {
            return std::stoi(Trim(json.substr(colonPos + 1, endPos - colonPos - 1)));
        }
        catch (...)
        {
            return fallback;
        }
    }

    ClientConfig Config::LoadClientConfig(const std::string& path)
    {
        ClientConfig config;
        const auto json = ReadFile(path);
        if (json.empty())
        {
            return config;
        }

        config.ServerIp = ReadJsonString(json, "server_ip", config.ServerIp);
        config.ServerPort = ReadJsonInt(json, "server_port", config.ServerPort);
        config.TelemetryFile = ReadJsonString(json, "telemetry_file", config.TelemetryFile);
        config.ClientName = ReadJsonString(json, "client_name", config.ClientName);
        config.SendIntervalMs = ReadJsonInt(json, "send_interval_ms", config.SendIntervalMs);
        return config;
    }

    ServerConfig Config::LoadServerConfig(const std::string& path)
    {
        ServerConfig config;
        const auto json = ReadFile(path);
        if (json.empty())
        {
            return config;
        }

        config.BindIp = ReadJsonString(json, "bind_ip", config.BindIp);
        config.ListenPort = ReadJsonInt(json, "listen_port", config.ListenPort);
        config.MaxClients = ReadJsonInt(json, "max_clients", config.MaxClients);
        config.LogFile = ReadJsonString(json, "log_file", config.LogFile);
        config.StatsFile = ReadJsonString(json, "stats_file", config.StatsFile);
        return config;
    }
}
