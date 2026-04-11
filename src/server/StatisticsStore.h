#pragma once

#include "../shared/FlightStatistics.h"
#include <unordered_map>
#include <string>
#include <mutex>

namespace FleetTelemetry
{
    class StatisticsStore
    {
    public:
        StatisticsStore() = default;
        ~StatisticsStore() = default;

        StatisticsStore(const StatisticsStore&) = delete;
        StatisticsStore& operator=(const StatisticsStore&) = delete;

        bool AppendFlightCsv(const std::string& filePath, const FlightStatistics& stats);
        bool SaveSnapshotCsv(const std::string& filePath, const std::unordered_map<std::string, FlightStatistics>& stats);

    private:
        bool AppendFlightCsvSync(const std::string& filePath, const FlightStatistics& stats);

        mutable std::mutex m_fileMutex;
    };
}
