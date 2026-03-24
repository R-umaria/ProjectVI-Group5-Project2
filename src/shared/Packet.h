#pragma once

#include "TelemetryRecord.h"
#include <string>

namespace FleetTelemetry
{
    class Packet
    {
    public:
        static std::string Serialize(const TelemetryRecord& record);
        static bool Deserialize(const std::string& packetText, TelemetryRecord& outRecord);
    };
}
