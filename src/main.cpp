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
    // client.setReceiveTimeout()
        
    Transaction *transaction_manager = new Transaction(client);

    if (!transaction_manager->connect() ) {
        Log(LogLevel::ERROR, "connect failed. cancelling operation");
        // exit(EXIT_FAILURE);
    }

    if (!transaction_manager->send_data("hello world") ) {
        Log(LogLevel::ERROR, "data sending failed. cancelling operation");
        // exit(EXIT_FAILURE);
    }
    if (!transaction_manager->disconnect() ) {
        exit(EXIT_FAILURE);
        // Log(LogLevel::ERROR, "disconnect failed. cancelling operation");
    }

    // Example usage of SlowPackage
    // SlowPackage pkg;
    // pkg.sid = {std::byte(0x01), std::byte(0x02), std::byte(0x03), std::byte(0x04),
    //            std::byte(0x05), std::byte(0x06), std::byte(0x07), std::byte(0x08),
    //            std::byte(0x09), std::byte(0x0A), std::byte(0x0B), std::byte(0x0C),
    //            std::byte(0x0D), std::byte(0x0E), std::byte(0x0F), std::byte(0x10)};
    // pkg.sttl = 1234567; // Example TTL value
    // pkg.flag_connect = true;
    // pkg.flag_revive = false;
    // pkg.flag_ack = true;
    // pkg.flag_accept_reject = false;
    // pkg.flag_mb = true;
    // pkg.seqnum = 987654321;
    // pkg.acknum = 123456789;
    // pkg.window = 1024;
    // pkg.fid = 1;
    // pkg.fo = 2;

    // auto serializedData = pkg.serialize();
    
    // // Deserialize the data back into a SlowPackage object
    // SlowPackage* deserializedPkg = SlowPackage::deserialize(serializedData);
    
    // // Print the string representation of the deserialized package
    // if (deserializedPkg) {
    //     std::cout << deserializedPkg->toString() << std::endl;
    //     delete deserializedPkg; // Clean up
    // } else {
    //     std::cerr << "Failed to deserialize package." << std::endl;
    // }

    return 0;
}