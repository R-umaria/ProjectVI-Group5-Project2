#pragma once

#include "../shared/TelemetryRecord.h"
#include <fstream>
#include <string>
#include <vector>

namespace FleetTelemetry
{
    class TelemetryReader
    {
    public:
        bool Open(const std::string& filePath);
        bool ReadNextLine(std::string& outLine);
        void Close();
        bool IsOpen() const;

    private:
        std::ifstream m_input;
    };
}