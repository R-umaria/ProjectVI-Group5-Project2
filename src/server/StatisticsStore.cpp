#include "StatisticsStore.h"
#include <fstream>
#include <filesystem>

namespace FleetTelemetry
{
    bool StatisticsStore::SaveCsv(const std::string& filePath, const std::unordered_map<std::string, FlightStatistics>& stats) const
    {
        std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
        std::ofstream output(filePath);
        if (!output)
        {
            return false;
        }

        output << "AircraftId,StartTimestamp,EndTimestamp,StartFuel,EndFuel,FuelConsumed,AverageFuelConsumptionPerSample,SampleCount\n";
        for (const auto& [_, value] : stats)
        {
            output << value.AircraftId << ','
                   << value.StartTimestamp << ','
                   << value.EndTimestamp << ','
                   << value.StartFuel << ','
                   << value.EndFuel << ','
                   << value.FuelConsumed << ','
                   << value.AverageFuelConsumptionPerSample << ','
                   << value.SampleCount << '\n';
        }

        return true;
    }
}
