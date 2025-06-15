#include <iostream>
#include <cstddef>
#include "slow_package.hpp"

int main() {
    // Example usage of SlowPackage
    SlowPackage pkg;
    pkg.sid = {std::byte(0x01), std::byte(0x02), std::byte(0x03), std::byte(0x04),
               std::byte(0x05), std::byte(0x06), std::byte(0x07), std::byte(0x08),
               std::byte(0x09), std::byte(0x0A), std::byte(0x0B), std::byte(0x0C),
               std::byte(0x0D), std::byte(0x0E), std::byte(0x0F), std::byte(0x10)};
    pkg.sttl = 1234567; // Example TTL value
    pkg.flag_connect = true;
    pkg.flag_revive = false;
    pkg.flag_ack = true;
    pkg.flag_accept_reject = false;
    pkg.flag_mb = true;
    pkg.seqnum = 987654321;
    pkg.acknum = 123456789;
    pkg.window = 1024;
    pkg.fid = 1;
    pkg.fo = 2;

    auto serializedData = pkg.serialize();
    
    // Deserialize the data back into a SlowPackage object
    SlowPackage* deserializedPkg = SlowPackage::deserialize(serializedData);
    
    // Print the string representation of the deserialized package
    if (deserializedPkg) {
        std::cout << deserializedPkg->toString() << std::endl;
        delete deserializedPkg; // Clean up
    } else {
        std::cerr << "Failed to deserialize package." << std::endl;
    }

    return 0;
}