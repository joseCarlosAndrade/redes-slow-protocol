#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include <mutex>
#include<vector>
#include<thread>
#include "udp_client.hpp"
#include "slow_package.hpp"

// placeholders (DELETE WHEN IMPLEMENTED) - WONT COMPILE YET BC THEYLL BE IMPLEMENTED
// enum class PackageType {CONNECT, SETUP, ACK, ACCEPT_REJECT, MORE_BITS};

// class SlowPackage {

//     public: 

//         SlowPackage();
//         ~SlowPackage();
//         // std::string serialize();
//         // static SlowPackage deserialize(std::byte data, size_t size);
//         std::string uuid;
//         std::string ip;
//         int port;
//         uint32_t sttl;
//         bool accepted_rejected;
//         uint32_t seqnum;
//         uint32_t acknum;
//         PackageType type;
//         bool revive;
        
// };

// class Client {
//     public:
//         Client();
         
//         ~Client();
//         void send_data(SlowPackage package);
//         SlowPackage receive_data();
// };

// -------------------------

enum class ConnectionStatus {OFFLINE, CONNECTED, EXPIRED};
class Transaction {
    public:
        Transaction(UdpClient* client);
        ~Transaction();

        // transaction functions
        
        // sends a connect and awaits a setup. Returns true if accepted, false if rejected
        bool connect();
        
        // sends data flow (to be implemented)
        bool send_data(std::string data, bool revive = false, int attempts_left = 5);
        
        // sends a disconnect to the server
        bool disconnect();

        // verifies if the connection is still valid (according to the expiration time)
        bool connection_still_alive();

        ConnectionStatus connection_status;

    private:
        UdpClient *client;
        std::array<std::byte, 16> session_uuid; // 16 bytes
        std::string session_ip; // bytes 
        int session_port;

        // session data
        std::chrono::time_point<std::chrono::steady_clock> session_expiration;

        uint32_t current_seqnum; // package number

        std::vector<SlowPackage> receiver_buffer; // keeps everything received from the server
        std::mutex buffer_mtx;
        std::mutex connection_status_mtx;

        // checks the buffer (thread safe) for a specific acknum and type package;
        // if it finds, returns true, removes the package from the buffer and sets package to the found value
        bool check_buffer_for_data(SlowPackage::PackageType type, uint32_t acknum, SlowPackage* package);

        std::thread listener_thread;
        void listen_to_incoming_data();
};