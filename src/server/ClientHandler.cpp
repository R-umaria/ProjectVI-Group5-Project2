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

        const bool crashed = !receiveError.empty();

        if (crashed)
        {
            m_logger.Error("Client crashed/disconnected for " +
                (aircraftId.empty() ? m_remoteAddress : aircraftId) +
                ": " + receiveError);
        }
        else
        {
            m_logger.Info("Client disconnected normally for " +
                (aircraftId.empty() ? m_remoteAddress : aircraftId));
        }

        if (!aircraftId.empty())
            {
                FlightStatistics finalStatistics;

                if (m_sessionManager.CompleteFlight(aircraftId, crashed, finalStatistics))
                {
                    if (crashed)
                    {
                        m_logger.Info(
                            "Partial flight saved for crashed client " + aircraftId +
                            " | Samples: " + std::to_string(finalStatistics.SampleCount) +
                            " | Fuel: " + std::to_string(finalStatistics.FuelConsumed));
                    }
                    else
                    {
                        m_logger.Info(
                            "Flight completed for " + aircraftId +
                            " | Avg fuel/hr: " + std::to_string(finalStatistics.AverageFuelConsumptionPerHour));
                    }
                }
                else
                {
                    m_logger.Error("Failed to finalize flight for " + aircraftId);
                }
        }

        Network::ShutdownSocket(m_clientSocket);
        Network::CloseSocket(m_clientSocket);
        m_clientSocket = InvalidSocket;
        m_logger.Info("Client session closed for " + (aircraftId.empty() ? m_remoteAddress : aircraftId));
    }
}
