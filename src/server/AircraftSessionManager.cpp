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

    bool AircraftSessionManager::CompleteFlight(const std::string& aircraftId, bool crashed, FlightStatistics& outStatistics)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_processor.FinalizeFlight(aircraftId, crashed, outStatistics))
        {
            return false;
        }

        //UPDATING THE CULMATIVE FLIGHT HISTORY
        auto& history = m_aircraftHistory[aircraftId];
        history.AircraftId = aircraftId;
        history.FlightCount += 1;
        history.CumulativeFuelConsumed += outStatistics.FuelConsumed;
        history.CumulativeTotalSeconds += outStatistics.TotalFlightSeconds;
        if (history.CumulativeTotalSeconds > 0.0)
        {
            const double avgPerSec = history.CumulativeFuelConsumed / history.CumulativeTotalSeconds;
            history.OverallAverageFuelConsumptionPerHour = avgPerSec * 3600.0;
        }
        outStatistics.FlightCount = history.FlightCount;
        outStatistics.CumulativeFuelConsumed = history.CumulativeFuelConsumed;
        outStatistics.CumulativeTotalSeconds = history.CumulativeTotalSeconds;
        outStatistics.OverallAverageFuelConsumptionPerHour = history.OverallAverageFuelConsumptionPerHour;



        if (!m_statisticsStore.AppendFlightCsv(m_statsFilePath, outStatistics))
        {
            m_logger.Error("Failed to persist flight statistics for " + aircraftId);
        }

        return true;
    }

	//fix for memory issues where if a client disconnects without completing the flight, clear it from the map to prevent mem leaks
    void AircraftSessionManager::LostConnectionFlight(const std::string& aircraftId)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        FlightStatistics discarded;
        m_processor.FinalizeFlight(aircraftId, true, discarded);// clears it from the map
    }

    std::unordered_map<std::string, FlightStatistics> AircraftSessionManager::GetActiveStatisticsSnapshot() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_processor.GetCurrentStatistics();
    }
}
