#pragma once

#include <string>
#include "../shared/TelemetryRecord.h"

namespace FleetTelemetry
{
    class PacketReceiver
    {
    public:
        bool ParseIncoming(const std::string& packetText, TelemetryRecord& record) const;
    };
}
