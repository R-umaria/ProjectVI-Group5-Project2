/*!
 * @file PacketBuilder.cpp
 * @brief Builds packets from telemetry data.
 *
 * This module converts telemetry records into a serialized format that can be sent to the server.
 */
#include "PacketBuilder.h"

namespace FleetTelemetry{

    std::string PacketBuilder::Build(const TelemetryRecord& record) const{

        //Check AircraftId is empty
        if (record.AircraftId.empty()){
            return "";
        }

        //Check record -> Timestamp is empty
        if (record.Timestamp.empty()){
            return "";
        }

        // Fuel must be positive value
        if (record.FuelQuantity < 0.0){
            return "";
        }

        return Packet::Serialize(record);
    }
}