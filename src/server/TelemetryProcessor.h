#pragma once

#include "../shared/TelemetryRecord.h"
#include "../shared/FlightStatistics.h"
#include <unordered_map>

namespace FleetTelemetry
{
    class TelemetryProcessor
    {
    public:
        void Process(const TelemetryRecord& record);
        std::unordered_map<std::string, FlightStatistics> GetCurrentStatistics() const;

    private:
        std::unordered_map<std::string, FlightStatistics> m_stats;
    };
}
