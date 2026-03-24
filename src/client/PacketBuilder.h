#pragma once

#include "../shared/Packet.h"

namespace FleetTelemetry
{
    class PacketBuilder
    {
    public:
        std::string Build(const TelemetryRecord& record) const;
    };
}
