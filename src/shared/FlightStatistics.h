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
        double CurrentFuelConsumptionPerSecond = 0.0;
        double AverageFuelConsumptionPerSecond = 0.0;
        double AverageFuelConsumptionPerHour = 0.0;
        double TotalFlightSeconds = 0.0;
        int SampleCount = 0;
        bool Completed = false;
        bool PartialFlight = false;
        std::string TerminationReason;
        //so that we can continue to track the average for 
        //the same airplane over multiple flights
        int FlightCount = 0;                          // how many completed flights for this plane
        double CumulativeFuelConsumed = 0.0;          // total fuel across all flights (of same plane)
        double CumulativeTotalSeconds = 0.0;          // total flight time over all flights (of same plane)
        double OverallAverageFuelConsumptionPerHour = 0.0;  //over all flights (of same plane)

    };
}
