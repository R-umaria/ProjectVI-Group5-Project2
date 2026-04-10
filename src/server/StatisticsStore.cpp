#include "StatisticsStore.h"
#include <fstream>
#include <filesystem>

namespace FleetTelemetry
{
    namespace
    {
        void WriteHeader(std::ofstream& output)
        {
            output << "AircraftId,StartTimestamp,EndTimestamp,StartFuel,EndFuel,"
                "FuelConsumed,CurrentFuelConsumptionPerSecond,AverageFuelConsumptionPerSecond,"
                "AverageFuelConsumptionPerHour,TotalFlightSeconds,SampleCount,Completed,"
                "FlightCount,CumulativeFuelConsumed,CumulativeTotalSeconds,OverallAverageFuelConsumptionPerHour\n";
        }

        void WriteStatsRow(std::ofstream& output, const FlightStatistics& value)
        {
            output << value.AircraftId << ','
                   << value.StartTimestamp << ','
                   << value.EndTimestamp << ','
                   << value.StartFuel << ','
                   << value.EndFuel << ','
                   << value.FuelConsumed << ','
                   << value.CurrentFuelConsumptionPerSecond << ','
                   << value.AverageFuelConsumptionPerSecond << ','
                   << value.AverageFuelConsumptionPerHour << ','
                   << value.TotalFlightSeconds << ','
                   << value.SampleCount << ','
                   << (value.Completed ? "true" : "false") << ','
                   << value.FlightCount << ','
                   << value.CumulativeFuelConsumed << ','
                   << value.CumulativeTotalSeconds << ','
                   << value.OverallAverageFuelConsumptionPerHour << '\n';
        }
    }

    StatisticsStore::StatisticsStore()
    {
        m_writerThread = std::thread(&StatisticsStore::WriterLoop, this);
    }

    StatisticsStore::~StatisticsStore()
    {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_stopRequested = true;
        }
        m_queueCondition.notify_all();

        if (m_writerThread.joinable())
        {
            m_writerThread.join();
        }
    }

    bool StatisticsStore::AppendFlightCsv(const std::string& filePath, const FlightStatistics& stats)
    {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_pendingWrites.push_back(PendingFlightWrite{ filePath, stats });
        }
        m_queueCondition.notify_one();
        return true;
    }

    void StatisticsStore::WriterLoop()
    {
        while (true)
        {
            PendingFlightWrite pendingWrite;

            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCondition.wait(lock, [&]()
                {
                    return m_stopRequested || !m_pendingWrites.empty();
                });

                if (m_pendingWrites.empty())
                {
                    if (m_stopRequested)
                    {
                        break;
                    }
                    continue;
                }

                pendingWrite = std::move(m_pendingWrites.front());
                m_pendingWrites.pop_front();
            }

            AppendFlightCsvSync(pendingWrite.FilePath, pendingWrite.Stats);
        }
    }

    bool StatisticsStore::AppendFlightCsvSync(const std::string& filePath, const FlightStatistics& stats)
    {
        std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
        const bool fileExists = std::filesystem::exists(filePath) && std::filesystem::file_size(filePath) > 0;

        std::ofstream output(filePath, std::ios::app);
        if (!output)
        {
            return false;
        }

        if (!fileExists)
        {
            WriteHeader(output);
        }

        WriteStatsRow(output, stats);
        return true;
    }

    bool StatisticsStore::SaveSnapshotCsv(const std::string& filePath, const std::unordered_map<std::string, FlightStatistics>& stats)
    {
        std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
        std::ofstream output(filePath);
        if (!output)
        {
            return false;
        }

        WriteHeader(output);
        for (const auto& entry : stats)
        {
            WriteStatsRow(output, entry.second);
        }

        return true;
    }
}
