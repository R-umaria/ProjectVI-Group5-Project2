#pragma once

#include "../shared/TelemetryRecord.h"
#include <string>

namespace FleetTelemetry
{
    class TelemetryParser
    {
    public:
        bool ParseLine(const std::string& line,
            const std::string& aircraftId,
            TelemetryRecord& outRecord) const;

    private:
        std::string Trim(const std::string& value) const;
    };
}