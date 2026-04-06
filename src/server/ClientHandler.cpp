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
                m_logger.Error("Malformed packet discarded from " + m_remoteAddress + ": " + packetText);
                continue;
            }

            if (aircraftId.empty())
            {
                aircraftId = record.AircraftId;
                m_logger.Info("Receiving telemetry for aircraft " + aircraftId + " from " + m_remoteAddress);
            }

            m_sessionManager.AcceptRecord(record);
            ++processedPackets;
            if (processedPackets == 1 || processedPackets % 100 == 0)
            {
                m_logger.Info("Processed packets for " + record.AircraftId + ": " + std::to_string(processedPackets));
            }
        }

        if (!receiveError.empty())
        {
            m_logger.Error("Connection ended for " + (aircraftId.empty() ? m_remoteAddress : aircraftId) + ": " + receiveError);
        }

        if (!aircraftId.empty())
        {
            FlightStatistics finalStatistics;
            if (m_sessionManager.CompleteFlight(aircraftId, finalStatistics))
            {
                m_logger.Info(
                    "Flight completed for " + aircraftId +
                    " (flight #" + std::to_string(finalStatistics.FlightCount) + ")" +
                    ". This flight avg fuel/hr = " + std::to_string(finalStatistics.AverageFuelConsumptionPerHour) +
                    ". Overall avg fuel/hr (for all flights this plane has done) = " + std::to_string(finalStatistics.OverallAverageFuelConsumptionPerHour));
            }
        }

        Network::ShutdownSocket(m_clientSocket);
        Network::CloseSocket(m_clientSocket);
        m_clientSocket = InvalidSocket;
        m_logger.Info("Client session closed for " + (aircraftId.empty() ? m_remoteAddress : aircraftId));
    }
}
