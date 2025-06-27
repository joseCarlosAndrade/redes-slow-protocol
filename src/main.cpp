#include <iostream>
#include <cstddef>
#include "slow_package.hpp"
#include "logger.hpp"
#include "udp_client.hpp"
#include "transaction.hpp"

int main() {
    Log(LogLevel::INFO, "starting application");

    std::string host = "142.93.184.175";
    int port = 7033;

    UdpClient* client = new UdpClient(host, port);
    client->setupConnection();
    client->setReceiveTimeout(0, 10);
        
    Transaction *transaction_manager = new Transaction(client);

    if (!transaction_manager->connect() ) {
        Log(LogLevel::ERROR, "connect failed. cancelling operation");
        exit(EXIT_FAILURE);
    }

    Log(LogLevel::INFO, "connected to server. sending data");

    if (!transaction_manager->send_data("hello world") ) {
        Log(LogLevel::ERROR, "data sending failed. cancelling operation");
        exit(EXIT_FAILURE);
    }

    Log(LogLevel::INFO, "data sent successfully. disconnecting");
    
    if (!transaction_manager->disconnect() ) {
        Log(LogLevel::ERROR, "disconnect failed. cancelling operation");
        exit(EXIT_FAILURE);
    }

    Log(LogLevel::INFO, "disconnected from server. sending more data");

    if (!transaction_manager->send_data("hello world again", true) ) {
        Log(LogLevel::ERROR, "data sending failed. cancelling operation");
        exit(EXIT_FAILURE);
    }

    Log(LogLevel::INFO, "data with revive sent successfully. disconnecting again");
    
    if (!transaction_manager->disconnect() ) {
        exit(EXIT_FAILURE);
        Log(LogLevel::ERROR, "disconnect failed. cancelling operation");
    }

    Log(LogLevel::INFO, "application finished successfully");
    return 0;
}