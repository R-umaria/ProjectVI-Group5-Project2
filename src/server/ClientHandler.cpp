#include "ClientHandler.h"
#include "../shared/Network.h"

namespace FleetTelemetry
{
    ClientHandler::ClientHandler(SocketHandle clientSocket,
        std::string remoteAddress,
        AircraftSessionManager& sessionManager,
        Logger& logger) :
        m_clientSocket(clientSocket),
        m_remoteAddress(std::move(remoteAddress)),
        m_sessionManager(sessionManager),
        m_logger(logger)
    {
    }

    void ClientHandler::Run()
    {
        std::string packetText;
        std::string receiveError;
        std::string aircraftId;
        int processedPackets = 0;
        int malformedPackets = 0;

        m_logger.Info("Client connected from " + m_remoteAddress);

        while (m_packetReceiver.ReceiveNextLine(m_clientSocket, packetText, &receiveError))
        {
            if (packetText.empty())
            {
                continue;
            }

            TelemetryRecord record;
            if (!m_packetReceiver.ParseIncoming(packetText, record))
            {
                ++malformedPackets;
                if (malformedPackets <= 3)
                {
                    m_logger.Error("Malformed packet discarded from " + m_remoteAddress + ": " + packetText);
                }
                continue;
            }

            if (aircraftId.empty())
            {
                aircraftId = record.AircraftId;
                m_logger.Info("Receiving telemetry for aircraft " + aircraftId + " from " + m_remoteAddress);
            }

            m_sessionManager.AcceptRecord(record);
            ++processedPackets;
        }

        if (!receiveError.empty())
        {
            m_logger.Error("Connection ended for " + (aircraftId.empty() ? m_remoteAddress : aircraftId) + ": " + receiveError);
        }

        if (malformedPackets > 3)
        {
            m_logger.Error("Additional malformed packets suppressed for "
                + (aircraftId.empty() ? m_remoteAddress : aircraftId)
                + ": " + std::to_string(malformedPackets - 3));
        }

        if (!aircraftId.empty() && processedPackets > 0)
        {
            FlightStatistics finalStatistics;
            if (m_sessionManager.CompleteFlight(aircraftId, finalStatistics))
            {
                m_logger.Info(
                    "Flight completed for " + aircraftId +
                    " (flight #" + std::to_string(finalStatistics.FlightCount) + ")" +
                    ". Packets processed = " + std::to_string(processedPackets) +
                    ". This flight avg fuel/hr = " + std::to_string(finalStatistics.AverageFuelConsumptionPerHour) +
                    ". Overall avg fuel/hr = " + std::to_string(finalStatistics.OverallAverageFuelConsumptionPerHour) +
                    ". Active sessions remaining = " + std::to_string(m_sessionManager.GetActiveSessionCount()));
            }
        }

        Network::ShutdownSocket(m_clientSocket);
        Network::CloseSocket(m_clientSocket);
        m_clientSocket = InvalidSocket;
        m_logger.Info("Client session closed for " + (aircraftId.empty() ? m_remoteAddress : aircraftId));
    }
}
