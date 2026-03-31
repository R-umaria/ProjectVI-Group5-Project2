#pragma once

#include <string>
#include <ctime>
#include <cstdio>

namespace FleetTelemetry
{
    namespace TimeUtils
    {
        inline bool TryParseTelemetryTimestamp(const std::string& timestampText, std::time_t& outTime)
        {
            int month = 0;
            int day = 0;
            int year = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;

            if (std::sscanf(timestampText.c_str(), "%d_%d_%d %d:%d:%d", &month, &day, &year, &hour, &minute, &second) == 6)
            {
                std::tm timeInfo{};
                timeInfo.tm_year = year - 1900;
                timeInfo.tm_mon = month - 1;
                timeInfo.tm_mday = day;
                timeInfo.tm_hour = hour;
                timeInfo.tm_min = minute;
                timeInfo.tm_sec = second;
                timeInfo.tm_isdst = -1;
                outTime = std::mktime(&timeInfo);
                return outTime != static_cast<std::time_t>(-1);
            }

            if (std::sscanf(timestampText.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6)
            {
                std::tm timeInfo{};
                timeInfo.tm_year = year - 1900;
                timeInfo.tm_mon = month - 1;
                timeInfo.tm_mday = day;
                timeInfo.tm_hour = hour;
                timeInfo.tm_min = minute;
                timeInfo.tm_sec = second;
                timeInfo.tm_isdst = -1;
                outTime = std::mktime(&timeInfo);
                return outTime != static_cast<std::time_t>(-1);
            }

            return false;
        }
    }
}
