#pragma once

#include "../shared/FlightStatistics.h"
#include <unordered_map>
#include <string>

namespace FleetTelemetry
{
    class StatisticsStore
    {
    public:
        bool SaveCsv(const std::string& filePath, const std::unordered_map<std::string, FlightStatistics>& stats) const;
    };
}
