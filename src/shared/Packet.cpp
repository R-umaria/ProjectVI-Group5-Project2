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
        const std::size_t firstComma = packetText.find(',');
        if (firstComma == std::string::npos)
        {
            return false;
        }

        const std::size_t secondComma = packetText.find(',', firstComma + 1);
        if (secondComma == std::string::npos)
        {
            return false;
        }

        outRecord.AircraftId = Trim(packetText.substr(0, firstComma));
        outRecord.Timestamp = Trim(packetText.substr(firstComma + 1, secondComma - firstComma - 1));

        if (outRecord.AircraftId.empty() || outRecord.Timestamp.empty())
        {
            return false;
        }

        try
        {
            outRecord.FuelQuantity = std::stod(Trim(packetText.substr(secondComma + 1)));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}
