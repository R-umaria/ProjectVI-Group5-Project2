#include "Logger.h"
#include <ctime>

namespace FleetTelemetry
{
    Logger::Logger(std::string filePath) :
        m_filePath(std::move(filePath))
    {
        std::filesystem::create_directories(std::filesystem::path(m_filePath).parent_path());
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
        std::lock_guard<std::mutex> lock(m_mutex);

        std::ofstream output(m_filePath, std::ios::app);
        if (!output)
        {
            return;
        }

        std::time_t now = std::time(nullptr);
        std::tm localTime{};
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localTime = *std::localtime(&now);
#endif
        output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
               << " [" << level << "] "
               << message << std::endl;
    }
}
