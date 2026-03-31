#pragma once

#include "../shared/TelemetryRecord.h"
#include "../shared/FlightStatistics.h"
#include <unordered_map>
#include <ctime>

namespace FleetTelemetry
{
    class TelemetryProcessor
    {
    public:
        void Process(const TelemetryRecord& record);
        bool FinalizeFlight(const std::string& aircraftId, FlightStatistics& outStatistics);
        std::unordered_map<std::string, FlightStatistics> GetCurrentStatistics() const;

    private:
        struct FlightState
        {
            FlightStatistics Statistics;
            std::time_t PreviousSampleTime = 0;
            double PreviousFuelQuantity = 0.0;
            bool HasPreviousSample = false;
        };

        std::unordered_map<std::string, FlightState> m_activeFlights;
    };
}
