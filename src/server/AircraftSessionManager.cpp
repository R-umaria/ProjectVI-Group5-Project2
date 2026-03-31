#include "AircraftSessionManager.h"

namespace FleetTelemetry
{
    AircraftSessionManager::AircraftSessionManager(std::string statsFilePath, Logger& logger) :
        m_statsFilePath(std::move(statsFilePath)),
        m_logger(logger)
    {
    }

    void AircraftSessionManager::AcceptRecord(const TelemetryRecord& record)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_processor.Process(record);
    }

    bool AircraftSessionManager::CompleteFlight(const std::string& aircraftId, FlightStatistics& outStatistics)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_processor.FinalizeFlight(aircraftId, outStatistics))
        {
            return false;
        }

        if (!m_statisticsStore.AppendFlightCsv(m_statsFilePath, outStatistics))
        {
            m_logger.Error("Failed to persist flight statistics for " + aircraftId);
        }

        return true;
    }

    std::unordered_map<std::string, FlightStatistics> AircraftSessionManager::GetActiveStatisticsSnapshot() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_processor.GetCurrentStatistics();
    }
}
