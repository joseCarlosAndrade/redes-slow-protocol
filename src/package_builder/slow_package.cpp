#include "../include/slow_package.hpp"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>    
#include <cstdint>
SlowPackage::SlowPackage() {
    // Initialize members with default values if needed

    // setting all defaul sid to 0
    this->sid.fill(std::byte{0});

    this->sttl = 0;

    this->flag_connect = false;
    this->flag_revive = false;
    this->flag_ack = false;
    this->flag_accept_reject = false;
    this->flag_mb = false;
    this->seqnum = 0;
    this->acknum = 0;
    this->window = 0;
    this->fid = 0;
    this->fo = 0;
    // data does not need allocation, its an empty vector

    this->type = SlowPackage::RAW;
}

SlowPackage::~SlowPackage() {
     // Clean up resources if necessary    
}

std::vector<std::byte> SlowPackage::serialize() {
    // Convert the SlowPackage object into a byte array

    // session UUID (0-127)
    std::vector<std::byte> byteArray(32);
    std::copy(sid.begin(), sid.end(), byteArray.begin());
    
    // Session TTL (128 - 154)
    // flag bits (155 - 160), 
    std::vector<std::byte> ttlFlagBytes;
    uint32_t sttlAndFlags = (sttl & 0x07FFFFFF); // 27 bits for sttl
    sttlAndFlags <<= 5;
    if (flag_connect) {
        sttlAndFlags |= (1u << 0); // Set bit 27 for flag_connect
    }
    if (flag_revive) {
        sttlAndFlags |= (1u << 1); // Set bit 28 for flag_revive
    }
    if (flag_ack) {
        sttlAndFlags |= (1u << 2); // Set bit 29 for flag_ack
    }
    if (flag_accept_reject) {
        sttlAndFlags |= (1u << 3); // Set bit 30 for flag_accept_reject
    }
    if (flag_mb) {
        sttlAndFlags |= (1u << 4); // Set bit 31 for flag_mb
    }
    // Write as little endian (least significant byte first)
    for (int i = 0; i < 4; ++i) {
        ttlFlagBytes.push_back(static_cast<std::byte>((sttlAndFlags >> (8 * i)) & 0xFF));
    }
    std::copy(ttlFlagBytes.begin(), ttlFlagBytes.end(), byteArray.begin() + 16); // 16 offset

    // seqnum (161 - 192)
    std::vector<std::byte> seqnumBytes;
    for (int i = 0; i < 4; ++i) {
        seqnumBytes.push_back(static_cast<std::byte>((seqnum >> (8 * i)) & 0xFF));
    }
    std::copy(seqnumBytes.begin(), seqnumBytes.end(), byteArray.begin() + 20); // 20 offset
    // acknum (193 - 224)
    std::vector<std::byte> acknumBytes;
    for (int i = 0; i < 4; ++i) {
        acknumBytes.push_back(static_cast<std::byte>((acknum >> (8 * i)) & 0xFF));
    }
    std::copy(acknumBytes.begin(), acknumBytes.end(), byteArray.begin() + 24); // 24 offset
    // window (225 - 240)
    std::vector<std::byte> windowBytes;
    for (int i = 0; i < 2; ++i) {
        windowBytes.push_back(static_cast<std::byte>((window >> (8 * i)) & 0xFF));
    }
    std::copy(windowBytes.begin(), windowBytes.end(), byteArray.begin() + 28); // 28 offset
    // fid (241 - 248)
    byteArray[30] = static_cast<std::byte>(fid); // 30 offset
    // fo (249 - 256)
    byteArray[31] = static_cast<std::byte>(fo); // 31 offset
    // data (257 - end)
    if (!data.empty()) {
        byteArray.insert(byteArray.end(), data.begin(), data.end());
    }
    return byteArray;
}
SlowPackage* SlowPackage::deserialize(std::vector<std::byte> data) {
    // Convert the byte array or string back into a SlowPackage object
    if (data.size() < 32) {
        std::cerr << "Data too short to deserialize into SlowPackage." << std::endl;
        return nullptr; // Not enough data to deserialize
    }
    SlowPackage* pkg = new SlowPackage();
    // session UUID (0-127)
    std::copy(data.begin(), data.begin() + 16, pkg->sid.begin());
    // Session TTL and flags (128 - 160)
    uint32_t sttlAndFlags = 0;
    for (int i = 0; i < 4; ++i) {
        sttlAndFlags |= (static_cast<uint32_t>(data[16 + i]) << (8 * i));
    }
    pkg->sttl = sttlAndFlags & 0x07FFFFFF; // Extract sttl (27 bits)
    pkg->flag_connect = (sttlAndFlags & (1u << 27)) != 0;
    pkg->flag_revive = (sttlAndFlags & (1u << 28)) != 0;
    pkg->flag_ack = (sttlAndFlags & (1u << 29)) != 0;   
    pkg->flag_accept_reject = (sttlAndFlags & (1u << 30)) != 0;
    pkg->flag_mb = (sttlAndFlags & (1u << 31)) != 0;
    // seqnum (161 - 192)
    pkg->seqnum = 0;
    for (int i = 0; i < 4; ++i) {
        pkg->seqnum |= (static_cast<uint32_t>(data[20 + i]) << (8 * i));
    }
    // acknum (193 - 224)
    pkg->acknum = 0;    
    for (int i = 0; i < 4; ++i) {
        pkg->acknum |= (static_cast<uint32_t>(data[24 + i]) << (8 * i));
    }
    // window (225 - 240)
    pkg->window = 0;
    for (int i = 0; i < 2; ++i) {
        pkg->window |= (static_cast<uint16_t>(data[28 + i]) << (8 * i));
    }
    // fid (241 - 248)
    pkg->fid = static_cast<uint8_t>(data[30]);
    // fo (249 - 256)
    pkg->fo = static_cast<uint8_t>(data[31]);
    // data (257 - end)
    pkg->data.clear();
    if (data.size() > 32) {
        pkg->data.insert(pkg->data.end(), data.begin() + 32, data.end());
    }

    
    return pkg;
}
std::string SlowPackage::toString() {
    // Implement a string representation of the package for debugging
    std::ostringstream oss;
    oss << "SlowPackage: {\n\tsid: ";
    for (size_t i = 0; i < sid.size(); ++i) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << static_cast<int>(std::to_integer<uint8_t>(sid[i]));
        if (i != sid.size() - 1) oss << " ";
    }

    oss << ",\n sttl: " << std::dec << sttl
        << ",\n \tflags: { connect: " << flag_connect
        << ",\n \t\trevive: " << flag_revive
        << ",\n \t\tack: " << flag_ack
        << ",\n \t\taccept_reject: " << flag_accept_reject
        << ",\n \t\tmb: " << flag_mb
        << " \n\t}, seqnum: " << std::dec << seqnum
        << ",\n \tacknum: " << std::dec << acknum
        << ",\n \twindow: " << std::dec << window
        << ",\n \tfid: " << static_cast<int>(fid)
        << ",\n \tfo: " << static_cast<int>(fo)
        << ",\n \tdata size: " << data.size()
        << " \n}";
    return oss.str();
}