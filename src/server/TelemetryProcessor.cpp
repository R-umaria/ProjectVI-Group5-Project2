#include "TelemetryProcessor.h"
#include "../shared/TimeUtils.h"
#include <algorithm>

namespace FleetTelemetry
{
    void TelemetryProcessor::Process(const TelemetryRecord& record)
    {
        auto& flightState = m_activeFlights[record.AircraftId];
        auto& stats = flightState.Statistics;

        if (stats.SampleCount == 0)
        {
            stats.AircraftId = record.AircraftId;
            stats.StartTimestamp = record.Timestamp;
            stats.EndTimestamp = record.Timestamp;
            stats.StartFuel = record.FuelQuantity;
            stats.EndFuel = record.FuelQuantity;
            stats.SampleCount = 1;

            std::time_t parsedTime = 0;
            if (TimeUtils::TryParseTelemetryTimestamp(record.Timestamp, parsedTime))
            {
                flightState.PreviousSampleTime = parsedTime;
                flightState.HasPreviousSample = true;
            }
            flightState.PreviousFuelQuantity = record.FuelQuantity;
            return;
        }

        stats.EndTimestamp = record.Timestamp;
        stats.EndFuel = record.FuelQuantity;
        stats.SampleCount += 1;

        std::time_t currentSampleTime = 0;
        const bool parsedCurrentTime = TimeUtils::TryParseTelemetryTimestamp(record.Timestamp, currentSampleTime);

        if (flightState.HasPreviousSample && parsedCurrentTime)
        {
            const double deltaSeconds = std::max(0.0, std::difftime(currentSampleTime, flightState.PreviousSampleTime));
            const double deltaFuel = std::max(0.0, flightState.PreviousFuelQuantity - record.FuelQuantity);

            if (deltaSeconds > 0.0)
            {
                stats.TotalFlightSeconds += deltaSeconds;
                stats.FuelConsumed += deltaFuel;
                stats.CurrentFuelConsumptionPerSecond = deltaFuel / deltaSeconds;
                stats.AverageFuelConsumptionPerSecond =
                    stats.TotalFlightSeconds > 0.0 ? stats.FuelConsumed / stats.TotalFlightSeconds : 0.0;
                stats.AverageFuelConsumptionPerHour = stats.AverageFuelConsumptionPerSecond * 3600.0;
            }
        }
        else
        {
            stats.FuelConsumed = std::max(0.0, stats.StartFuel - stats.EndFuel);
        }

        if (parsedCurrentTime)
        {
            flightState.PreviousSampleTime = currentSampleTime;
            flightState.HasPreviousSample = true;
        }
        flightState.PreviousFuelQuantity = record.FuelQuantity;
    }

    bool TelemetryProcessor::FinalizeFlight(const std::string& aircraftId, bool crashed, FlightStatistics& outStatistics)
    {
        const auto it = m_activeFlights.find(aircraftId);
        if (it == m_activeFlights.end())
        {
            return false;
        }

        outStatistics = it->second.Statistics;

        outStatistics.FuelConsumed =
            std::max(outStatistics.FuelConsumed,
                std::max(0.0, outStatistics.StartFuel - outStatistics.EndFuel));

        if (outStatistics.TotalFlightSeconds > 0.0)
        {
            outStatistics.AverageFuelConsumptionPerSecond =
                outStatistics.FuelConsumed / outStatistics.TotalFlightSeconds;

            outStatistics.AverageFuelConsumptionPerHour =
                outStatistics.AverageFuelConsumptionPerSecond * 3600.0;
        }

        if (crashed)
        {
            outStatistics.Completed = false;
            outStatistics.PartialFlight = true;
            outStatistics.TerminationReason = "ClientCrash";
        }
        else
        {
            outStatistics.Completed = true;
            outStatistics.PartialFlight = false;
            outStatistics.TerminationReason = "NormalDisconnect";
        }

        m_activeFlights.erase(it);
        return true;
    }

    std::unordered_map<std::string, FlightStatistics> TelemetryProcessor::GetCurrentStatistics() const
    {
        std::unordered_map<std::string, FlightStatistics> snapshot;
        for (const auto& entry : m_activeFlights)
        {
            snapshot.emplace(entry.first, entry.second.Statistics);
        }
        return snapshot;
    }
}
