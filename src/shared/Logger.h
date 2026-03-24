#pragma once

#include "Common.h"

namespace FleetTelemetry
{
    class Logger
    {
    public:
        explicit Logger(std::string filePath);
        void Info(const std::string& message);
        void Error(const std::string& message);

    private:
        void Write(const std::string& level, const std::string& message);

        std::string m_filePath;
        std::mutex m_mutex;
    };
}
