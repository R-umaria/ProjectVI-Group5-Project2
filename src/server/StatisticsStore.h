#pragma once

#include "../shared/FlightStatistics.h"
#include <unordered_map>
#include <string>
#include <condition_variable>
#include <deque>
#include <thread>
#include <mutex>

namespace FleetTelemetry
{
    class StatisticsStore
    {
    public:
        StatisticsStore();
        ~StatisticsStore();

        StatisticsStore(const StatisticsStore&) = delete;
        StatisticsStore& operator=(const StatisticsStore&) = delete;

        bool AppendFlightCsv(const std::string& filePath, const FlightStatistics& stats);
        bool SaveSnapshotCsv(const std::string& filePath, const std::unordered_map<std::string, FlightStatistics>& stats);

    private:
        struct PendingFlightWrite
        {
            std::string FilePath;
            FlightStatistics Stats;
        };

        void WriterLoop();
        bool AppendFlightCsvSync(const std::string& filePath, const FlightStatistics& stats);

        std::mutex m_queueMutex;
        std::condition_variable m_queueCondition;
        std::deque<PendingFlightWrite> m_pendingWrites;
        std::thread m_writerThread;
        bool m_stopRequested = false;
    };
}
