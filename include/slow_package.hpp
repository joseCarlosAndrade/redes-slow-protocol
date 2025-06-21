#ifndef SLOW_PACKAGE_H
#define SLOW_PACKAGE_H
#include <string>
#include <stdint.h>
#include <cstddef>
#include <array>
#include <vector>



class SlowPackage {
     public:
      // enum for package types, defined in spcification
      enum PackageType {
          CONNECT, 
          SETUP,
          DATA,
          ACK,
          RAW // no data, default
        };
      // Data fields, check project specification for more information
      std::array<std::byte, 16> sid;
      uint32_t sttl;
      // Package flags
      bool flag_connect;
      bool flag_revive;
      bool flag_ack;
      bool flag_accept_reject;
      bool flag_mb;
      uint32_t seqnum;
      uint32_t acknum;
      uint16_t window;
      uint8_t fid;
      uint8_t fo;
      std::vector<std::byte> data;
      PackageType type;
      // Public methods
      SlowPackage(); //Constructor
      ~SlowPackage(); //Destructor
      std::vector<std::byte> serialize(); // Serializer
      static SlowPackage* deserialize(std::vector<std::byte> data); // static deserializer
      std::string toString(); // For debugging purposes
};


#endif // SLOW_PACKAGE_H
