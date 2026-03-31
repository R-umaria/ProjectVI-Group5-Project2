#include "TelemetryParser.h"
#include <sstream>
#include <cctype>

namespace FleetTelemetry
{
    std::string TelemetryParser::Trim(const std::string& value) const
    {
        size_t start = 0;
        while (start < value.size() &&
            std::isspace(static_cast<unsigned char>(value[start])))
        {
            ++start;
        }

        size_t end = value.size();
        while (end > start &&
            std::isspace(static_cast<unsigned char>(value[end - 1])))
        {
            --end;
        }

        return value.substr(start, end - start);
    }

    bool TelemetryParser::ParseLine(const std::string& line,
        const std::string& aircraftId,
        TelemetryRecord& outRecord) const
    {
        const std::string trimmedLine = Trim(line);

        if (trimmedLine.empty())
            return false;

        // Skip header
        if (trimmedLine.find("FUEL TOTAL QUANTITY") != std::string::npos)
            return false;

        std::stringstream ss(trimmedLine);

        std::string timestampText;
        std::string fuelText;

        if (!std::getline(ss, timestampText, ','))
            return false;

        if (!std::getline(ss, fuelText, ','))
            return false;

        timestampText = Trim(timestampText);
        fuelText = Trim(fuelText);

        if (timestampText.empty() || fuelText.empty())
            return false;

        try
        {
            double fuel = std::stod(fuelText);

            if (fuel < 0.0)
                return false;

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