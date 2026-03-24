#pragma once

#include <string>

namespace FleetTelemetry
{
    struct TelemetryRecord
    {
        std::string AircraftId;
        std::string Timestamp;
        double FuelQuantity = 0.0;
    };
}
