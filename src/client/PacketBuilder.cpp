#include "PacketBuilder.h"

namespace FleetTelemetry
{
    std::string PacketBuilder::Build(const TelemetryRecord& record) const
    {
        return Packet::Serialize(record);
    }
}
