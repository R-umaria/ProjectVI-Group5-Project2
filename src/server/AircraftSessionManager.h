#pragma once

#include "TelemetryProcessor.h"
#include "../shared/TelemetryRecord.h"

namespace FleetTelemetry
{
    class AircraftSessionManager
    {
    public:
        void AcceptRecord(const TelemetryRecord& record);
        const TelemetryProcessor& GetProcessor() const;

    private:
        TelemetryProcessor m_processor;
    };
}
