#pragma once

#include <string>

namespace FleetTelemetry
{
    struct FlightStatistics
    {
        std::string AircraftId;
        std::string StartTimestamp;
        std::string EndTimestamp;
        double StartFuel = 0.0;
        double EndFuel = 0.0;
        double FuelConsumed = 0.0;
        double AverageFuelConsumptionPerSample = 0.0;
        int SampleCount = 0;
    };
}
