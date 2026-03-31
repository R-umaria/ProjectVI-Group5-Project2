#include "TelemetryParser.h"
#include "../shared/Common.h"
#include <sstream>
#include <cctype>

namespace FleetTelemetry
{
    std::string TelemetryParser::Trim(const std::string& value) const
    {
        return FleetTelemetry::Trim(value);
    }

    bool TelemetryParser::ParseLine(const std::string& line,
        const std::string& aircraftId,
        TelemetryRecord& outRecord) const
    {
        std::string workingLine = Trim(line);
        if (workingLine.empty())
        {
            return false;
        }

        const std::string header = "FUEL TOTAL QUANTITY";
        const auto headerPosition = workingLine.find(header);
        if (headerPosition != std::string::npos)
        {
            const auto commaPosition = workingLine.find(',', headerPosition + header.size());
            if (commaPosition == std::string::npos)
            {
                return false;
            }
            workingLine = Trim(workingLine.substr(commaPosition + 1));
        }

        std::stringstream ss(workingLine);
        std::string timestampText;
        std::string fuelText;

        if (!std::getline(ss, timestampText, ','))
        {
            return false;
        }

        if (!std::getline(ss, fuelText, ','))
        {
            return false;
        }

        timestampText = Trim(timestampText);
        fuelText = Trim(fuelText);

        if (timestampText.empty() || fuelText.empty())
        {
            return false;
        }

        try
        {
            const double fuel = std::stod(fuelText);
            if (fuel < 0.0)
            {
                return false;
            }

            outRecord.AircraftId = aircraftId;
            outRecord.Timestamp = timestampText;
            outRecord.FuelQuantity = fuel;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}
