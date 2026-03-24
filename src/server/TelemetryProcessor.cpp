#include "TelemetryProcessor.h"

namespace FleetTelemetry
{
    void TelemetryProcessor::Process(const TelemetryRecord& record)
    {
        auto& stats = m_stats[record.AircraftId];
        if (stats.SampleCount == 0)
        {
            stats.AircraftId = record.AircraftId;
            stats.StartTimestamp = record.Timestamp;
            stats.StartFuel = record.FuelQuantity;
        }

        stats.EndTimestamp = record.Timestamp;
        stats.EndFuel = record.FuelQuantity;
        stats.SampleCount += 1;
        stats.FuelConsumed = stats.StartFuel - stats.EndFuel;
        stats.AverageFuelConsumptionPerSample = stats.SampleCount > 0 ? stats.FuelConsumed / stats.SampleCount : 0.0;
    }

    std::unordered_map<std::string, FlightStatistics> TelemetryProcessor::GetCurrentStatistics() const
    {
        return m_stats;
    }
}
