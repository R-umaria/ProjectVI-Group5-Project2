/*!
 * @file PacketReceiver.cpp
 * @brief Receives and processes incoming packets.
 *
 * This module reads data from sockets, extracts complete lines and converts them into telemetry records.
 */
#include "PacketReceiver.h"
#include "../shared/Packet.h"
#include "../shared/Network.h"

namespace FleetTelemetry
{
    bool PacketReceiver::ReceiveNextLine(SocketHandle socketHandle, std::string& outLine, std::string* errorMessage)
    {
        outLine.clear();

        while (true)
        {
            const auto newlinePosition = m_buffer.find('\n');
            if (newlinePosition != std::string::npos)
            {
                outLine = m_buffer.substr(0, newlinePosition);
                if (!outLine.empty() && outLine.back() == '\r')
                {
                    outLine.pop_back();
                }
                m_buffer.erase(0, newlinePosition + 1);
                return true;
            }

            char buffer[4096] = {};
            const int bytesReceived = recv(socketHandle, buffer, static_cast<int>(sizeof(buffer)), 0);
            if (bytesReceived > 0)
            {
                m_buffer.append(buffer, static_cast<size_t>(bytesReceived));
                continue;
            }

            if (bytesReceived == 0)
            {
                if (!m_buffer.empty())
                {
                    outLine = m_buffer;
                    m_buffer.clear();
                    return true;
                }
                return false;
            }

            if (errorMessage != nullptr)
            {
                *errorMessage = Network::GetLastErrorString();
            }
            return false;
        }
    }

    bool PacketReceiver::ParseIncoming(const std::string& packetText, TelemetryRecord& record) const
    {
        return Packet::Deserialize(packetText, record);
    }
}
