#include "PacketReceiver.h"
#include "../shared/Packet.h"

namespace FleetTelemetry
{
    bool PacketReceiver::ParseIncoming(const std::string& packetText, TelemetryRecord& record) const
    {
        return Packet::Deserialize(packetText, record);
    }
}
