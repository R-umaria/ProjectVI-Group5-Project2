#pragma once

#include "Common.h"

namespace FleetTelemetry
{
    class Logger
    {
    public:
        explicit Logger(std::string filePath, bool mirrorToConsole = true, bool flushOnWrite = false);
        ~Logger();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void Info(const std::string& message);
        void Error(const std::string& message);

    private:
        void Write(const std::string& level, const std::string& message);

        std::string m_filePath;
        std::ofstream m_output;
        std::mutex m_mutex;
        bool m_mirrorToConsole = true;
        bool m_flushOnWrite = false;
    };
}
