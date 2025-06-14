#include "../../include/package_builder.hpp"
#include "../../include/slow_package.hpp"

#include <array>
#include <cstddef>

// Return a connect package, receives the window buffer remaining size
SlowPackage conectPackage(uint16_t window) {
    auto pkg = new SlowPackage();
    pkg->type = SlowPackage::PackageType::CONNECT;
    pkg->sid.fill(std::byte(0)); // Initialize sid with zeros
    pkg->sttl = 0; // Set TTL to 0 for connect package  
    pkg->flag_connect = true; // Set connect flag
    pkg->flag_revive = false; // Set revive flag to false
    pkg->flag_ack = false; // Set ack flag to false
    pkg->flag_accept_reject = false; // Set accept/reject flag to false
    pkg->flag_mb = false; // Set mb flag to false
    pkg->seqnum = 0; // Set sequence number to 0
    pkg->acknum = 0; // Set acknowledgment number to 0
    pkg->window = window; // Set the window size
    pkg->fid = 0; // Set fid to 0
    pkg->fo = 0; // Set fo to 0
    pkg->data.clear(); // Clear data vector
    return *pkg; // Return the package
}

// Return a disconnect package, requires session data
SlowPackage disconnectPackage(std::array<std::byte, 16> sid, uint32_t sttl, uint32_t seqnum, uint32_t acknum){
    auto pkg = new SlowPackage();
    pkg->type = SlowPackage::PackageType::DISCONNECT;
    pkg->sid = sid; // Set session ID
    pkg->sttl = sttl; // Set session TTL
    pkg->flag_connect = true; // Set connect flag to true
    pkg->flag_revive = true; // Set revive flag to true (connect and revive means disconnect)
    pkg->flag_ack = true; // Set ack flag to true
    pkg->flag_accept_reject = false; // Set accept/reject flag to false
    pkg->flag_mb = false; // Set mb flag to false
    pkg->seqnum = seqnum; // Set sequence number
    pkg->acknum = acknum; // Set acknowledgment number
    pkg->window = 0; // Set window size to 0
    pkg->fid = 0; // Set fid to 0
    pkg->fo = 0; // Set fo to 0
    pkg->data.clear(); // Clear data vector
    return *pkg; // Return the package
}

// NOTE: when using this function, keep in mind the project documentation
// is not clear at all about seqnum ,acknum and fid, this code
// implements their logic bases on asssumptions
//
// Given some data, returns a vector of SlowPackages
// fragmented by the max size
std::vector<SlowPackage> fragmentedDataPackages(std::array<std::byte, 16> sid, uint32_t sttl, uint32_t seqnum, 
    uint32_t acknum, uint16_t window, uint8_t fid, std::vector<std::byte> data) {
        uint32_t unsentBytes = data.size();
        uint32_t seqnumInc = 0;
        uint8_t fo = 0; // Fragment offset
        std::vector<SlowPackage> packages; // Vector to hold the packages
        while (unsentBytes > 1440) {
            auto pkg = new SlowPackage();
            pkg->type = SlowPackage::PackageType::DATA;
            pkg->sid = sid; // Set session ID
            pkg->sttl = sttl; // Set session TTL
            pkg->flag_connect = false; // Set connect flag to false
            pkg->flag_revive = false; // Set revive flag to false
            pkg->flag_ack = true; // Set ack flag to true
            pkg->flag_accept_reject = false; // Set accept/reject flag to false
            pkg->flag_mb = true; // Set mb flag to true
            pkg->seqnum = seqnum++; // Set sequence number
            pkg->acknum = acknum; // Set acknowledgment number
            pkg->window = window--; // Set window size
            pkg->fid = fid; // Set fid
            pkg->fo = fo++; // increment fragment offset

            uint32_t dataSize = std::min(unsentBytes, static_cast<uint32_t>(1440)); // Max data size per package is 1440 bytes
            pkg->data.insert(pkg->data.end(), data.begin() + (data.size() - unsentBytes), data.begin() + (data.size() - unsentBytes + dataSize));
            
            unsentBytes -= dataSize;

            packages.push_back(*pkg); // Add the package to the vector
        }
        auto pkg = new SlowPackage();
            pkg->type = SlowPackage::PackageType::DATA;
            pkg->sid = sid; // Set session ID
            pkg->sttl = sttl; // Set session TTL
            pkg->flag_connect = false; // Set connect flag to false
            pkg->flag_revive = false; // Set revive flag to false
            pkg->flag_ack = true; // Set ack flag to true
            pkg->flag_accept_reject = false; // Set accept/reject flag to false
            pkg->flag_mb = false; // Set mb flag to true
            pkg->seqnum = seqnum++; // Set sequence number
            pkg->acknum = acknum; // Set acknowledgment number
            pkg->window = window--; // Set window size
            pkg->fid = fid; // Set fid
            pkg->fo = fo++; // increment fragment offset

            uint32_t dataSize = std::min(unsentBytes, static_cast<uint32_t>(1440)); // Max data size per package is 1440 bytes
            pkg->data.insert(pkg->data.end(), data.begin() + (data.size() - unsentBytes), data.begin() + (data.size() - unsentBytes + dataSize));
            packages.push_back(*pkg); // Add the package to the vector
        return packages;
    }


// Same as data packages, but the first packag
std::vector<SlowPackage> fragmentedRevivePackages(std::array<std::byte, 16> sid, uint32_t sttl, uint32_t seqnum, 
    uint32_t acknum, uint16_t window, uint8_t fid, std::vector<std::byte> data) {
        auto pkgs = fragmentedDataPackages(sid, sttl, seqnum, acknum, window, fid, data);
        // Modify the first package to be a revive package
        if (!pkgs.empty()) {
            pkgs[0].flag_revive = true;
        }
        return pkgs;
    }


// clasifies a response package based on flags
SlowPackage::PackageType classifyResponsePackage(const SlowPackage& pkg) {
    if (!pkg.flag_ack) {
        return SlowPackage::PackageType::SETUP;
    }
    return SlowPackage::PackageType::ACK;
}
