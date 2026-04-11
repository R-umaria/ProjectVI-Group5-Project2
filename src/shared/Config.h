#pragma once

#include "Common.h"

namespace FleetTelemetry
{
    struct ClientConfig
    {
        std::string ServerIp = "127.0.0.1";
        int ServerPort = 54000;
        std::string TelemetryFile = "data/sample/Telemetry_Data_Sample.txt";
        std::string ClientName = "AIR";
        int SendIntervalMs = 250;
    };

    struct ServerConfig
    {
        std::string BindIp = "0.0.0.0";
        int ListenPort = 54000;
        int MaxClients = 32;
        std::string LogFile = "output/logs/server/server.log";
        std::string StatsFile = "output/stats/flight_stats.csv";
    };

    class Config
    {
    public:
        static ClientConfig LoadClientConfig(const std::string& path);
        static ServerConfig LoadServerConfig(const std::string& path);

    private:
        static std::string ReadFile(const std::string& path);
        static std::string ReadJsonString(const std::string& json, const std::string& key, const std::string& fallback);
        static int ReadJsonInt(const std::string& json, const std::string& key, int fallback);
    };
}
