#pragma once

#include "../shared/TelemetryRecord.h"
#include <string>

namespace FleetTelemetry
{
    class TelemetryParser
    {
    public:
        bool IsValid(const TelemetryRecord& record) const;
        std::string Describe(const TelemetryRecord& record) const;
    };
}
