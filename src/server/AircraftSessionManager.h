#pragma once

#include "TelemetryProcessor.h"
#include "StatisticsStore.h"
#include "../shared/TelemetryRecord.h"
#include "../shared/Logger.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <cstddef>

namespace FleetTelemetry
{
    class AircraftSessionManager
    {
    public:
        AircraftSessionManager(std::string statsFilePath, Logger& logger);

        void AcceptRecord(const TelemetryRecord& record);
        bool CompleteFlight(const std::string& aircraftId, FlightStatistics& outStatistics);
        std::unordered_map<std::string, FlightStatistics> GetActiveStatisticsSnapshot() const;
        std::size_t GetActiveSessionCount() const;

    private:
        struct SessionEntry
        {
            mutable std::mutex Mutex;
            TelemetryProcessor::FlightState Flight;
        };

        struct AircraftHistoryEntry
        {
            int FlightCount = 0;
            double CumulativeFuelConsumed = 0.0;
            double CumulativeTotalSeconds = 0.0;
            double OverallAverageFuelConsumptionPerHour = 0.0;
        };

        std::shared_ptr<SessionEntry> FindOrCreateSession(const std::string& aircraftId);

        std::string m_statsFilePath;
        Logger& m_logger;
        TelemetryProcessor m_processor;
        StatisticsStore m_statisticsStore;

        mutable std::mutex m_sessionsMutex;
        std::unordered_map<std::string, std::shared_ptr<SessionEntry>> m_activeSessions;

        mutable std::mutex m_historyMutex;
        std::unordered_map<std::string, AircraftHistoryEntry> m_aircraftHistory;
    };
}
