#include "TelemetryParser.h"

namespace FleetTelemetry
{
    bool TelemetryParser::IsValid(const TelemetryRecord& record) const
    {
        return !record.AircraftId.empty() && !record.Timestamp.empty() && record.FuelQuantity >= 0.0;
    }

    std::string TelemetryParser::Describe(const TelemetryRecord& record) const
    {
        return record.AircraftId + " @ " + record.Timestamp + " fuel=" + std::to_string(record.FuelQuantity);
    }
}
