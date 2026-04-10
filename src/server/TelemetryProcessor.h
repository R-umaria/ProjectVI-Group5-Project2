#pragma once

#include "../shared/TelemetryRecord.h"
#include "../shared/FlightStatistics.h"
#include <ctime>

namespace FleetTelemetry
{
    class TelemetryProcessor
    {
    public:
        struct FlightState
        {
            FlightStatistics Statistics;
            std::time_t PreviousSampleTime = 0;
            double PreviousFuelQuantity = 0.0;
            bool HasPreviousSample = false;
        };

        void ProcessRecord(FlightState& flightState, const TelemetryRecord& record) const;
        void FinalizeFlight(FlightState& flightState, FlightStatistics& outStatistics) const;
    };
}
