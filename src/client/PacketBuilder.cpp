/*!
 * @file PacketBuilder.cpp
 * @brief Builds packets from telemetry data.
 *
 * This module converts telemetry records into a serialized format that can be sent to the server.
 */
#include "PacketBuilder.h"

namespace FleetTelemetry
{
    std::string PacketBuilder::Build(const TelemetryRecord& record) const
    {
        return Packet::Serialize(record);
    }
}
