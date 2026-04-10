#include "Logger.h"
#include <ctime>
#include <iostream>

namespace FleetTelemetry
{
    namespace
    {
        std::mutex g_loggerConsoleMutex;
    }

    Logger::Logger(std::string filePath, bool mirrorToConsole, bool flushOnWrite) :
        m_filePath(std::move(filePath)),
        m_mirrorToConsole(mirrorToConsole),
        m_flushOnWrite(flushOnWrite)
    {
        std::filesystem::create_directories(std::filesystem::path(m_filePath).parent_path());
        m_output.open(m_filePath, std::ios::app);
    }

    Logger::~Logger()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_output.is_open())
        {
            m_output.flush();
            m_output.close();
        }
    }

    void Logger::Info(const std::string& message)
    {
        Write("INFO", message);
    }

    void Logger::Error(const std::string& message)
    {
        Write("ERROR", message);
    }

    void Logger::Write(const std::string& level, const std::string& message)
    {
        std::ostringstream line;

        std::time_t now = std::time(nullptr);
        std::tm localTime{};
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localTime = *std::localtime(&now);
#endif

        line << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
             << " [" << level << "] "
             << message;

        const std::string renderedLine = line.str();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_output.is_open())
            {
                m_output << renderedLine << '\n';
                if (m_flushOnWrite || level == "ERROR")
                {
                    m_output.flush();
                }
            }
        }

        if (m_mirrorToConsole)
        {
            std::lock_guard<std::mutex> consoleLock(g_loggerConsoleMutex);
            std::cout << renderedLine << '\n';
        }
    }
}
