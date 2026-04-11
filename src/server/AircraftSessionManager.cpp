#include "AircraftSessionManager.h"
#include "../shared/PathUtils.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace FleetTelemetry
{
    namespace
    {
        std::string ExtractField(const std::string& line, std::size_t fieldIndex)
        {
            std::size_t start = 0;
            for (std::size_t current = 0; current < fieldIndex; ++current)
            {
                const std::size_t comma = line.find(',', start);
                if (comma == std::string::npos)
                {
                    return "";
                }
                start = comma + 1;
            }

            const std::size_t comma = line.find(',', start);
            return line.substr(start, comma == std::string::npos ? std::string::npos : (comma - start));
        }

        int ParseIntSafe(const std::string& text, int fallback)
        {
            try
            {
                return std::stoi(text);
            }
            catch (...)
            {
                return fallback;
            }
        }

        double ParseDoubleSafe(const std::string& text, double fallback)
        {
            try
            {
                return std::stod(text);
            }
            catch (...)
            {
                return fallback;
            }
        }
    }

    AircraftSessionManager::AircraftSessionManager(std::string statsFilePath, Logger& logger) :
        m_statsFilePath(std::move(statsFilePath)),
        m_logger(logger)
    {
    }

    AircraftSessionManager::SessionEntry* AircraftSessionManager::FindOrCreateSession(const std::string& aircraftId)
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        auto iterator = m_activeSessions.find(aircraftId);
        if (iterator != m_activeSessions.end())
        {
            return iterator->second.get();
        }

        auto session = std::make_unique<SessionEntry>();
        SessionEntry* sessionPtr = session.get();
        m_activeSessions.emplace(aircraftId, std::move(session));
        return sessionPtr;
    }

    std::string AircraftSessionManager::BuildHistoryFilePath() const
    {
        namespace fs = std::filesystem;
        fs::path statsPath(m_statsFilePath);
        if (!statsPath.has_parent_path())
        {
            statsPath = FleetTelemetry::PathUtils::ResolveWritablePath(m_statsFilePath);
        }

        return (statsPath.parent_path() / "aircraft_history.csv").string();
    }

    AircraftSessionManager::AircraftHistoryEntry AircraftSessionManager::LoadPersistedHistory(const std::string& aircraftId) const
    {
        std::ifstream input(BuildHistoryFilePath());
        if (!input)
        {
            return {};
        }

        std::string line;
        while (std::getline(input, line))
        {
            if (line.empty() || line.rfind("AircraftId,", 0) == 0)
            {
                continue;
            }

            if (ExtractField(line, 0) != aircraftId)
            {
                continue;
            }

            AircraftHistoryEntry history;
            history.FlightCount = ParseIntSafe(ExtractField(line, 1), 0);
            history.CumulativeFuelConsumed = ParseDoubleSafe(ExtractField(line, 2), 0.0);
            history.CumulativeTotalSeconds = ParseDoubleSafe(ExtractField(line, 3), 0.0);
            history.OverallAverageFuelConsumptionPerHour = ParseDoubleSafe(ExtractField(line, 4), 0.0);
            return history;
        }

        return {};
    }

    bool AircraftSessionManager::SavePersistedHistory(const std::string& aircraftId, const AircraftHistoryEntry& history)
    {
        namespace fs = std::filesystem;
        const fs::path historyPath = BuildHistoryFilePath();
        fs::create_directories(historyPath.parent_path());

        std::unordered_map<std::string, AircraftHistoryEntry> allHistory;
        std::ifstream input(historyPath);
        std::string line;
        while (std::getline(input, line))
        {
            if (line.empty() || line.rfind("AircraftId,", 0) == 0)
            {
                continue;
            }

            const std::string existingAircraftId = ExtractField(line, 0);
            if (existingAircraftId.empty())
            {
                continue;
            }

            AircraftHistoryEntry entry;
            entry.FlightCount = ParseIntSafe(ExtractField(line, 1), 0);
            entry.CumulativeFuelConsumed = ParseDoubleSafe(ExtractField(line, 2), 0.0);
            entry.CumulativeTotalSeconds = ParseDoubleSafe(ExtractField(line, 3), 0.0);
            entry.OverallAverageFuelConsumptionPerHour = ParseDoubleSafe(ExtractField(line, 4), 0.0);
            allHistory[existingAircraftId] = entry;
        }

        allHistory[aircraftId] = history;

        const fs::path tempPath = historyPath.string() + ".tmp";
        std::ofstream output(tempPath, std::ios::trunc);
        if (!output)
        {
            return false;
        }

        output << "AircraftId,FlightCount,CumulativeFuelConsumed,CumulativeTotalSeconds,OverallAverageFuelConsumptionPerHour\n";
        for (const auto& entry : allHistory)
        {
            output << entry.first << ','
                   << entry.second.FlightCount << ','
                   << entry.second.CumulativeFuelConsumed << ','
                   << entry.second.CumulativeTotalSeconds << ','
                   << entry.second.OverallAverageFuelConsumptionPerHour << '\n';
        }

        output.flush();
        if (!output)
        {
            return false;
        }

        std::error_code ec;
        fs::rename(tempPath, historyPath, ec);
        if (ec)
        {
            fs::remove(historyPath, ec);
            ec.clear();
            fs::rename(tempPath, historyPath, ec);
        }

        return !ec;
    }

    void AircraftSessionManager::MaybeCompactSessionStorageLocked()
    {
        if (m_activeSessions.empty())
        {
            std::unordered_map<std::string, std::unique_ptr<SessionEntry>> empty;
            m_activeSessions.swap(empty);
            return;
        }

        const std::size_t activeCount = m_activeSessions.size();
        const std::size_t bucketCount = m_activeSessions.bucket_count();
        if (bucketCount > (activeCount * 4))
        {
            m_activeSessions.rehash(activeCount * 2);
        }
    }

    void AircraftSessionManager::AcceptRecord(const TelemetryRecord& record)
    {
        SessionEntry* session = FindOrCreateSession(record.AircraftId);
        std::lock_guard<std::mutex> sessionLock(session->Mutex);
        m_processor.ProcessRecord(session->Flight, record);
    }

    bool AircraftSessionManager::CompleteFlight(const std::string& aircraftId, FlightStatistics& outStatistics)
    {
        std::unique_ptr<SessionEntry> session;
        {
            std::lock_guard<std::mutex> lock(m_sessionsMutex);
            const auto iterator = m_activeSessions.find(aircraftId);
            if (iterator == m_activeSessions.end())
            {
                return false;
            }

            session = std::move(iterator->second);
            m_activeSessions.erase(iterator);
            MaybeCompactSessionStorageLocked();
        }

        {
            std::lock_guard<std::mutex> sessionLock(session->Mutex);
            m_processor.FinalizeFlight(session->Flight, outStatistics);
        }

        {
            std::lock_guard<std::mutex> historyLock(m_historyFileMutex);
            AircraftHistoryEntry history = LoadPersistedHistory(aircraftId);
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

            if (!SavePersistedHistory(aircraftId, history))
            {
                m_logger.Error("Failed to persist aircraft history for " + aircraftId);
            }
        }

        if (!m_statisticsStore.AppendFlightCsv(m_statsFilePath, outStatistics))
        {
            m_logger.Error("Failed to queue flight statistics for " + aircraftId);
        }

        return true;
    }

    std::size_t AircraftSessionManager::GetActiveSessionCount() const
    {
        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        return m_activeSessions.size();
    }

    std::unordered_map<std::string, FlightStatistics> AircraftSessionManager::GetActiveStatisticsSnapshot() const
    {
        std::unordered_map<std::string, FlightStatistics> snapshot;

        std::lock_guard<std::mutex> lock(m_sessionsMutex);
        snapshot.reserve(m_activeSessions.size());
        for (const auto& entry : m_activeSessions)
        {
            std::lock_guard<std::mutex> sessionLock(entry.second->Mutex);
            snapshot.emplace(entry.first, entry.second->Flight.Statistics);
        }
        return snapshot;
    }
}
