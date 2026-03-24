#include "Packet.h"
#include "Common.h"

namespace FleetTelemetry
{
    std::string Packet::Serialize(const TelemetryRecord& record)
    {
        return record.AircraftId + "," + record.Timestamp + "," + std::to_string(record.FuelQuantity);
    }

    bool Packet::Deserialize(const std::string& packetText, TelemetryRecord& outRecord)
    {
        const auto parts = Split(packetText, ',');
        if (parts.size() != 3)
        {
            return false;
        }

        try
        {
            outRecord.AircraftId = Trim(parts[0]);
            outRecord.Timestamp = Trim(parts[1]);
            outRecord.FuelQuantity = std::stod(Trim(parts[2]));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}
