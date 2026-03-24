#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <filesystem>

namespace FleetTelemetry
{
    inline std::string Trim(const std::string& value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
        {
            return "";
        }

        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    inline std::vector<std::string> Split(const std::string& value, char delimiter)
    {
        std::vector<std::string> parts;
        std::stringstream stream(value);
        std::string item;

        while (std::getline(stream, item, delimiter))
        {
            parts.push_back(item);
        }

        return parts;
    }
}
