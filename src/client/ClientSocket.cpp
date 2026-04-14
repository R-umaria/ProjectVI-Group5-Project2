/*!
 * @file ClientSocket.cpp
 * @brief Handles TCP client socket operations such as connecting, sending data and disconnecting from the server.
 */
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ClientSocket.h"
#include "../shared/Network.h"
#include <iostream>
#include <string>

using namespace FleetTelemetry;

ClientSocket::~ClientSocket() {
    Disconnect();
}

bool ClientSocket::Connect(const std::string& hostname, int port, std::string* errorMessage) {

    Disconnect();

    if (!Network::Initialize(errorMessage)) {
        return false;
    }

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* listOfServerSocketAddr = nullptr;

    std::string portStr = std::to_string(port);

    int status = getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &listOfServerSocketAddr);
    if (status != 0)
    {
        if (errorMessage != nullptr)
        {
#ifdef _WIN32
            * errorMessage = "getaddrinfo failed (code: " + std::to_string(status) + ")";
#else
            * errorMessage = gai_strerror(status);
#endif
        }

        return false;
    }

    addrinfo* iterator = nullptr;

    std::string lastConnectError = "Unable to connect to server";

    for (iterator = listOfServerSocketAddr;
        iterator != nullptr;
        iterator = iterator->ai_next)
    {
        clientSocket = socket(
            iterator->ai_family,
            iterator->ai_socktype,
            iterator->ai_protocol
        );

        if (clientSocket == InvalidSocket) {
            lastConnectError = Network::GetLastErrorString(); 
            continue;
        }

        if (connect(clientSocket,
            iterator->ai_addr,
            static_cast<int>(iterator->ai_addrlen)) == 0) {

            break;
        }

        lastConnectError = Network::GetLastErrorString(); 

        Network::CloseSocket(clientSocket);
        clientSocket = InvalidSocket;
    }

    freeaddrinfo(listOfServerSocketAddr);

    if (iterator == nullptr) {

        if (errorMessage != nullptr) {
            *errorMessage = lastConnectError; 
        }

        return false;
    }

    isConnected = true;
    return true;
}

void ClientSocket::Disconnect() {

    if (clientSocket == InvalidSocket) {
        isConnected = false;
        return;
    }

    Network::ShutdownSocket(clientSocket);
    Network::CloseSocket(clientSocket);

    clientSocket = InvalidSocket;
    isConnected = false;
}

bool ClientSocket::IsConnected() const {
    return isConnected;
}
/*
bool ClientSocket::SendLine(const std::string& line, std::string* errorMessage) {
    if (!isConnected) {

        if (errorMessage != nullptr) {
            *errorMessage = "Socket is not connected properly";
        }

        return false;
    }

    return Network::SendAll(clientSocket, line + "\n", errorMessage);
}
*/

// To see if the packets are being sent
bool ClientSocket::SendLine(const std::string& line, std::string* errorMessage) {
    if (!isConnected) {

        if (errorMessage != nullptr) {
            *errorMessage = "Socket is not connected properly";
        }

        std::cout << "[ERROR] Socket is not connected properly\n";
        return false;
    }

    //static int counter = 0;
    int counter = 0;   
    ++counter;            

    //std::cout << "[SEND] " << line << std::endl;

    bool result = Network::SendAll(clientSocket, line + "\n", errorMessage);

    if (result){

        if (counter % 100 == 0)  {
            std::cout << "[INFO] Sent " << counter << " packets\n";
        }
    }

    else{
        std::cout << "[FAIL] Send package is failed!\n";
    }

    return result;
}