#include "Packet.h"
#include "Common.h"
#include <sstream>
#include <iomanip>

namespace FleetTelemetry
{
    std::string Packet::Serialize(const TelemetryRecord& record)
    {
        std::ostringstream stream;
        stream << record.AircraftId << ','
               << record.Timestamp << ','
               << std::fixed << std::setprecision(6)
               << record.FuelQuantity;
        return stream.str();
    }

    bool Packet::Deserialize(const std::string& packetText, TelemetryRecord& outRecord)
    {
        const auto parts = Split(packetText, ',');
        if (parts.size() < 3)
        {
            return false;
        }

        try
        {
            outRecord.AircraftId = Trim(parts[0]);
            outRecord.Timestamp = Trim(parts[1]);
            outRecord.FuelQuantity = std::stod(Trim(parts[2]));
            return !outRecord.AircraftId.empty() && !outRecord.Timestamp.empty();
        }
        catch (...)
        {
            return false;
        }
    }
}
