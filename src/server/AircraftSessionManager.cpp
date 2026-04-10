#include "AircraftSessionManager.h"
#include <vector>

namespace FleetTelemetry
{
    AircraftSessionManager::AircraftSessionManager(std::string statsFilePath, Logger& logger) :
        m_statsFilePath(std::move(statsFilePath)),
        m_logger(logger)
    {
    }

    std::shared_ptr<AircraftSessionManager::SessionEntry> AircraftSessionManager::FindOrCreateSession(const std::string& aircraftId)
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        auto iterator = m_activeSessions.find(aircraftId);
        if (iterator != m_activeSessions.end())
        {
            return iterator->second;
        }

        auto session = std::make_shared<SessionEntry>();
        m_activeSessions.emplace(aircraftId, session);
        return session;
    }

    void AircraftSessionManager::AcceptRecord(const TelemetryRecord& record)
    {
        const auto session = FindOrCreateSession(record.AircraftId);
        std::lock_guard<std::mutex> sessionLock(session->Mutex);
        m_processor.ProcessRecord(session->Flight, record);
    }

    bool AircraftSessionManager::CompleteFlight(const std::string& aircraftId, FlightStatistics& outStatistics)
    {
        std::shared_ptr<SessionEntry> session;
        {
            std::lock_guard<std::mutex> lock(m_sessionsMutex);
            const auto iterator = m_activeSessions.find(aircraftId);
            if (iterator == m_activeSessions.end())
            {
                return false;
            }

            session = iterator->second;
            m_activeSessions.erase(iterator);
        }

        {
            std::lock_guard<std::mutex> sessionLock(session->Mutex);
            m_processor.FinalizeFlight(session->Flight, outStatistics);
        }

        {
            std::lock_guard<std::mutex> historyLock(m_historyMutex);
            auto& history = m_aircraftHistory[aircraftId];
            history.FlightCount += 1;
            history.CumulativeFuelConsumed += outStatistics.FuelConsumed;
            history.CumulativeTotalSeconds += outStatistics.TotalFlightSeconds;
            if (history.CumulativeTotalSeconds > 0.0)
            {
                const double averagePerSecond = history.CumulativeFuelConsumed / history.CumulativeTotalSeconds;
                history.OverallAverageFuelConsumptionPerHour = averagePerSecond * 3600.0;
            }

            outStatistics.FlightCount = history.FlightCount;
            outStatistics.CumulativeFuelConsumed = history.CumulativeFuelConsumed;
            outStatistics.CumulativeTotalSeconds = history.CumulativeTotalSeconds;
            outStatistics.OverallAverageFuelConsumptionPerHour = history.OverallAverageFuelConsumptionPerHour;
        }

        if (!m_statisticsStore.AppendFlightCsv(m_statsFilePath, outStatistics))
        {
            m_logger.Error("Failed to queue flight statistics for " + aircraftId);
        }

        return true;
    }

    std::unordered_map<std::string, FlightStatistics> AircraftSessionManager::GetActiveStatisticsSnapshot() const
    {
        std::vector<std::pair<std::string, std::shared_ptr<SessionEntry>>> sessions;
        {
            std::lock_guard<std::mutex> lock(m_sessionsMutex);
            sessions.reserve(m_activeSessions.size());
            for (const auto& entry : m_activeSessions)
            {
                sessions.emplace_back(entry.first, entry.second);
            }
        }

        std::unordered_map<std::string, FlightStatistics> snapshot;
        snapshot.reserve(sessions.size());
        for (const auto& entry : sessions)
        {
            std::lock_guard<std::mutex> sessionLock(entry.second->Mutex);
            snapshot.emplace(entry.first, entry.second->Flight.Statistics);
        }
        return snapshot;
    }
}
