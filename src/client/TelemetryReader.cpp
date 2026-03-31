#include "TelemetryReader.h"
#include "TelemetryParser.h"

namespace FleetTelemetry
{
    bool TelemetryReader::Open(const std::string& filePath)
    {
        Close();
        m_input.open(filePath);
        return m_input.is_open();
    }

    bool TelemetryReader::ReadNextLine(std::string& outLine)
    {
        if (!m_input.is_open())
            return false;

        return static_cast<bool>(std::getline(m_input, outLine));
    }

    void TelemetryReader::Close()
    {
        if (m_input.is_open())
            m_input.close();
    }

    bool TelemetryReader::IsOpen() const
    {
        return m_input.is_open();
    }

    std::vector<TelemetryRecord> TelemetryReader::ReadAll(const std::string& filePath)
    {
        std::vector<TelemetryRecord> records;

        if (!Open(filePath))
        {
            return records;
        }

        TelemetryParser parser;
        std::string line;
        const std::string aircraftId = "AIR-001";

        while (ReadNextLine(line))
        {
            TelemetryRecord record;

            if (parser.ParseLine(line, aircraftId, record))
            {
                records.push_back(record);
            }
        }

        Close();
        return records;
    }
}