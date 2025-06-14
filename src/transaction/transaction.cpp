#include "transaction.hpp"
#include"logger.hpp"
#include<thread>

#define N_RETRIES 10

Transaction::Transaction(Client client) {
    this->client = &client;
}

Transaction::~Transaction() {
    this->client = nullptr;
}

bool Transaction::connection_still_alive() {
    return (std::chrono::steady_clock::now() < this->session_expiration);
}

bool Transaction::connect() {
    Log(LogLevel::INFO, "requesting connection");

    // send connect package
    int current_seq = 0;

    SlowPackage connect_package; //create connection package
    connect_package.seqnum = current_seq;
    this->client->send_data(connect_package);

    // receive setup data (containing sesstion and stuff)
    SlowPackage setup_data;
    bool found = false;
    int retries = N_RETRIES;

    while (retries-- > 0) {
        // if it finds, exit while
        if(this->check_buffer_for_data(PackageType::SETUP, 0, &setup_data)) {
            found = true;
            break;
        }

        // if it doesnt find, sleep for 100 ms and try again for N_RETRIES
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (!found) {
        // error
        Log(LogLevel::ERROR, "did not receive any setup msg from server");
        return false;
    }

    // found a setup, but it may be rejected

    if (!setup_data.accepted_rejected) {
        Log(LogLevel::WARNING, "connection rejected by server");
        return false;
    } 

    Log(LogLevel::INFO, "received setup response from server. Connection accepted");
    // save session data
    this->session_uuid = setup_data.uuid; // TODO: check on how it will be implemented
    this->session_ip = setup_data.ip;
    this->session_port = setup_data.port;
    this->current_seqnum = setup_data.seqnum;

    // set session expiration
    const uint32_t MAX_27BIT = 0x7FFFFFF;

    // (mask to 27 bits)
    auto received_sttl = setup_data.sttl & MAX_27BIT;

    std::chrono::milliseconds  ttl_duration(received_sttl); // converting to milliseconds

    this->session_expiration = std::chrono::steady_clock::now() + ttl_duration;
    return true;
}

bool Transaction::send_data(std::string data, bool revive) {
    Log(LogLevel::INFO, "sending data: '" + data + "'");

    // for now, limit the data size to 1440 bytes (1472 - 32 bytes from header)
    // TODO : implement fragmentation
    // TODO : implement window

    // complicated stuff to develop
}

bool Transaction::disconnect() {
    Log(LogLevel::INFO, "requesting disconnect");

    int seqnum = this->current_seqnum;

    SlowPackage disconnect_package; // to be implement
    disconnect_package.seqnum = seqnum;
    this->client->send_data(disconnect_package);

    SlowPackage response;
    bool found = false;
    int retries = N_RETRIES;

    while (retries-- > 0) {
        // if it finds, exit while
        if(this->check_buffer_for_data(PackageType::ACK, seqnum, &response)) {
            found = true;
            break;
        }

        // if it doesnt find, sleep for 100 ms and try again for N_RETRIES
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (!found) {
        Log(LogLevel::ERROR, "did not receive any ack from server");
        return false;
    }

    Log(LogLevel::INFO, "ack received. Successfully disconnected");

    return false;
}

bool Transaction::check_buffer_for_data(PackageType type, uint32_t acknum, SlowPackage* package) {

    this->buffer_mtx.lock();
    
    // checks if theres an ack with the matching acknum = seqnum
    int index = 0;
    for (auto pack : this->receiver_buffer) {
        // if its type and matches acknum, then it found
        if (pack.acknum == acknum && pack.type == type) {
            *package = pack;

            // removing this package from buffer (consuming it)
            this->receiver_buffer.erase(this->receiver_buffer.begin() + index);

            this->buffer_mtx.unlock();
            return true;
        }
        index++;
    }

    this->buffer_mtx.unlock();

    return false;
}