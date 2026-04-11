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

    bool StatisticsStore::AppendFlightCsv(const std::string& filePath, const FlightStatistics& stats)
    {
        std::lock_guard<std::mutex> lock(m_fileMutex);
        return AppendFlightCsvSync(filePath, stats);
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
        output.flush();
        return static_cast<bool>(output);
    }

    bool StatisticsStore::SaveSnapshotCsv(const std::string& filePath, const std::unordered_map<std::string, FlightStatistics>& stats)
    {
        std::lock_guard<std::mutex> lock(m_fileMutex);

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
        output.flush();
        return static_cast<bool>(output);
    }
}
