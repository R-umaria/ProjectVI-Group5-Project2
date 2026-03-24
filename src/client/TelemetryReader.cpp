#include "TelemetryReader.h"
#include "../shared/Common.h"
#include <fstream>

namespace FleetTelemetry
{
    std::vector<TelemetryRecord> TelemetryReader::ReadAll(const std::string& filePath) const
    {
        std::vector<TelemetryRecord> records;
        std::ifstream input(filePath);
        std::string line;

        while (std::getline(input, line))
        {
            if (Trim(line).empty())
            {
                continue;
            }

            const auto parts = Split(line, ',');
            if (parts.size() != 3)
            {
                continue;
            }

            try
            {
                TelemetryRecord record;
                record.AircraftId = Trim(parts[0]);
                record.Timestamp = Trim(parts[1]);
                record.FuelQuantity = std::stod(Trim(parts[2]));
                records.push_back(record);
            }
            catch (...)
            {
            }
        }

        return records;
    }
}
