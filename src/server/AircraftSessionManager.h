#pragma once

#include "TelemetryProcessor.h"
#include "StatisticsStore.h"
#include "../shared/TelemetryRecord.h"
#include "../shared/Logger.h"
#include <mutex>
#include <string>

namespace FleetTelemetry
{
    class AircraftSessionManager
    {
    public:
        AircraftSessionManager(std::string statsFilePath, Logger& logger);

        void AcceptRecord(const TelemetryRecord& record);
        bool CompleteFlight(const std::string& aircraftId, bool crashed, FlightStatistics& outStatistics);
        std::unordered_map<std::string, FlightStatistics> GetActiveStatisticsSnapshot() const;

    private:
        std::string m_statsFilePath;
        Logger& m_logger;
        mutable std::mutex m_mutex;
        TelemetryProcessor m_processor;
        StatisticsStore m_statisticsStore;

        //for the overall stats per plane with multiple flightds
        std::unordered_map<std::string, FlightStatistics> m_aircraftHistory;
    };
}
