#include "TelemetryReader.h"

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
        {
            return false;
        }

        return static_cast<bool>(std::getline(m_input, outLine));
    }

    void TelemetryReader::Close()
    {
        if (m_input.is_open())
        {
            m_input.close();
        }
    }

    bool TelemetryReader::IsOpen() const
    {
        return m_input.is_open();
    }
}