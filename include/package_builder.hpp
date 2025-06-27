#pragma once

#include<iostream>

#include <array>
#include <cstddef>
#include"slow_package.hpp"
#include <vector>

SlowPackage conectPackage(uint16_t window);

// Return a disconnect package, requires session data
SlowPackage disconnectPackage(std::array<std::byte, 16> sid, uint32_t sttl, uint32_t seqnum, uint32_t acknum);

// NOTE: when using this function, keep in mind the project documentation
// is not clear at all about seqnum ,acknum and fid, this code
// implements their logic bases on asssumptions
//
// Given some data, returns a vector of SlowPackages
// fragmented by the max size
std::vector<SlowPackage> fragmentedDataPackages(std::array<std::byte, 16> sid, uint32_t sttl, uint32_t seqnum, 
    uint32_t acknum, uint16_t window, uint8_t fid, std::vector<std::byte> data);

// Same as data packages, but the first packag
std::vector<SlowPackage> fragmentedRevivePackages(std::array<std::byte, 16> sid, uint32_t sttl, uint32_t seqnum, 
    uint32_t acknum, uint16_t window, uint8_t fid, std::vector<std::byte> data);

// clasifies a response package based on flags
SlowPackage::PackageType classifyResponsePackage(const SlowPackage& pkg) ;