#pragma once

#include <string>
#include "../shared/TelemetryRecord.h"
#include "../shared/Network.h"

namespace FleetTelemetry
{
    class PacketReceiver
    {
    public:
        bool ReceiveNextLine(SocketHandle socketHandle, std::string& outLine, std::string* errorMessage = nullptr);
        bool ParseIncoming(const std::string& packetText, TelemetryRecord& record) const;

    private:
        std::string m_buffer;
    };
}
