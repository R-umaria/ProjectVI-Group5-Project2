#pragma once

#include "../shared/TelemetryRecord.h"
#include <string>
#include <vector>

namespace FleetTelemetry
{
    class TelemetryReader
    {
    public:
        std::vector<TelemetryRecord> ReadAll(const std::string& filePath) const;
    };
}
