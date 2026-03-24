#include "AircraftSessionManager.h"

namespace FleetTelemetry
{
    void AircraftSessionManager::AcceptRecord(const TelemetryRecord& record)
    {
        m_processor.Process(record);
    }

    const TelemetryProcessor& AircraftSessionManager::GetProcessor() const
    {
        return m_processor;
    }
}
