#include "transaction.hpp"
#include "logger.hpp"
#include <thread>
#include "udp_client.hpp"
#include<string>

#define N_RETRIES 10
#define AWAIT_TIME_MS 100

Transaction::Transaction(UdpClient *client) {
    if  (client == nullptr) {
        Log(LogLevel::ERROR, "client is null");
        exit(EXIT_FAILURE);
    }

    this->client = client;

    this->connection_status_mtx.lock();
    this->connection_status = ConnectionStatus::OFFLINE;
    this->connection_status_mtx.unlock();
}

Transaction::~Transaction() {
    this->client = nullptr;

    
}

bool Transaction::connection_still_alive() {
    if (std::chrono::steady_clock::now() < this->session_expiration) {
        return true;
    }

    Log(LogLevel::WARNING, "[transaction] SESSION EXPIRED");

    this->connection_status_mtx.lock();
    // this->connection_status = ConnectionStatus::EXPIRED;
    this->connection_status_mtx.unlock();

    return false;
}

bool Transaction::connect() {
    Log(LogLevel::INFO, "[transaction] requesting connection");

    this->connection_status = ConnectionStatus::CONNECTING; 
    // spawns the listener thread
    this->listener_thread = std::thread(&Transaction::listen_to_incoming_data, this);

    if (this->connection_still_alive()) {
        Log(LogLevel::INFO, "connection already established. Skipping..");
        return true;
    }

    // send connect package
    // int current_seq = 0;

    // SlowPackage connect_package; //create connection package
    // connect_package.seqnum = current_seq;
    // this->client->send_data(connect_package);
    SlowPackage connect_package;
    connect_package.sid = {};
    connect_package.sttl = 0;
    connect_package.flag_connect = true;
    connect_package.seqnum = 0;
    connect_package.acknum = 0;
    connect_package.window = 256;
    connect_package.fid = 0;
    connect_package.fo = 0;
    // data is not initialized
    // this->client->send()

    // serializing
    auto data_bytes = connect_package.serialize();

    bool sent = false;
    // tries to send it 5 times
    for (int i = 0; i < 5; i++) {
        if (this->client->send_bytes(data_bytes)) {
            sent = true;
            break;
        }
    }

    if (!sent) {
        Log(LogLevel::ERROR, "[transaction] error sending connect package. Cancelling");
        return false;
    }

    Log(LogLevel::INFO, "[transaction] connect package sent successfully");

    // receive setup data (containing sesstion and stuff)
    SlowPackage setup_data;
    bool found = false;
    int retries = N_RETRIES;

    while (retries-- > 0) {
        // if it finds, exit while
        if(this->check_buffer_for_data(SlowPackage::SETUP, 0, &setup_data)) {
            found = true;
            break;
        }

        // if it doesnt find, sleep for 100 ms and try again for N_RETRIES
        std::this_thread::sleep_for(std::chrono::milliseconds(AWAIT_TIME_MS));
    }
    
    if (!found) {
        // error
        Log(LogLevel::ERROR, "[transaction] did not receive any setup msg from server");
        return false;
    }

    // found a setup, but it may be rejected

    if (!setup_data.flag_accept_reject) {
        Log(LogLevel::WARNING, "[transaction] connection rejected by server");
        return false;
    } 

    Log(LogLevel::INFO, "[transaction] received setup response from server. Connection accepted");
    // save session data
    this->session_uuid = setup_data.sid; // TODO: check on how it will be implemented
    this->current_seqnum = setup_data.seqnum;
    this->cuirrent_sttl = setup_data.sttl;
    // set session expiration
    const uint32_t MAX_27BIT = 0x7FFFFFF;

    // (mask duration to 27 bits)
    auto received_sttl = setup_data.sttl & MAX_27BIT;
    

    std::chrono::milliseconds ttl_duration(received_sttl); // converting to milliseconds

    this->session_expiration = std::chrono::steady_clock::now() + ttl_duration;
    
    this->connection_status_mtx.lock();
    this->connection_status = ConnectionStatus::CONNECTED;
    this->connection_status_mtx.unlock();



    return true;
}

bool Transaction::send_data(std::string data, bool revive, int attempts_left) {
    Log(LogLevel::INFO, "[transaction] sending data: '" + data + "'. Attempts left: " + std::to_string(attempts_left));

    if (this->connection_status != ConnectionStatus::CONNECTED) {
        Log(LogLevel::ERROR, "[transaction] failed to send data: not connected.");
        return false;
    }

    // for now, limit the data size to 1440 bytes (1472 - 32 bytes from header)
    // TODO : implement fragmentation
    // TODO : implement window

    int seqnum = this->current_seqnum;

    SlowPackage package_data; // to be implemented
   
    package_data.sid = this->session_uuid;
    package_data.sttl = this->cuirrent_sttl;
    package_data.flag_ack = false; // TODO: the specification requests for this flag to be set to 1, but it doenst work
    package_data.seqnum = seqnum;
    package_data.window = 256;


    if (revive) package_data.flag_revive = true;

    auto package_bytes = package_data.serialize();
    
    bool sent = false;

    for (int i = 0; i <5 ; i++ ) {
        if (this->client->send_bytes(package_bytes)) {
            sent = true;
            break;
        }
    }

    if (!sent) {
        Log(LogLevel::ERROR, "[transaction] could not send data package. Cancelling");
        return false;
    }

    SlowPackage ack_data;
    bool found = false;
    int retries = N_RETRIES;

    while (retries-- > 0) {
        if (this->check_buffer_for_data(SlowPackage::ACK, seqnum, &ack_data)) {
            found = true;
            break;
        } 
        std::this_thread::sleep_for(std::chrono::milliseconds(AWAIT_TIME_MS));
    }

    if (!found) {
        if (attempts_left <= 0) {
            Log(LogLevel::ERROR, "[transaction] attempts exhausted. No ack received from server. Giving up");
            return false;
        }

        // attempts to resend the data up to attempts_left times
        Log(LogLevel::ERROR, "did not receive ack from server. Retrying..  Attempts left: " + std::to_string(attempts_left));
        return this->send_data(data, revive, --attempts_left);
    }

    Log(LogLevel::INFO, "[transaction] ack received for data. Data successfully sent");

    // updating curernt seqnum accordingly
    this->current_seqnum = ack_data.seqnum;
    
    return true;

    // (complicated stuff here to develop later)
}

bool Transaction::disconnect() {
    Log(LogLevel::INFO, "[transaction] requesting disconnect");

    this->connection_status_mtx.lock();
    this->connection_status = ConnectionStatus::OFFLINE; // no matter if its successful or not, disconnect
    this->connection_status_mtx.lock();
    
    int seqnum = this->current_seqnum;

    SlowPackage disconnect_package; // to be implemented
    disconnect_package.sid = this->session_uuid;
    disconnect_package.sttl = this->cuirrent_sttl;
    disconnect_package.flag_ack = true;
    disconnect_package.flag_connect = true;
    disconnect_package.flag_revive = true;
    disconnect_package.acknum = 0; // TODO fix this
    disconnect_package.seqnum = seqnum;

    auto package_bytes = disconnect_package.serialize();
    this->client->send_bytes(package_bytes);

    SlowPackage response;
    bool found = false;
    int retries = N_RETRIES;

    while (retries-- > 0) {
        // if it finds, exit while
        if(this->check_buffer_for_data(SlowPackage::ACK, seqnum, &response)) {
            found = true;
            break;
        }

        // if it doesnt find, sleep for 100 ms and try again for N_RETRIES
        std::this_thread::sleep_for(std::chrono::milliseconds(AWAIT_TIME_MS));
    }
    
    if (!found) {
        Log(LogLevel::ERROR, "[transaction] did not receive any ack from server");
        return false;
    }

    Log(LogLevel::INFO, "[transaction] ack received. Successfully disconnected");

    return false;
}

bool Transaction::check_buffer_for_data(SlowPackage::PackageType type, uint32_t acknum, SlowPackage* package) {

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

void Transaction::listen_to_incoming_data() {
    Log(LogLevel::INFO, "[transaction] listening to incomming messages from server..");

    for (;;) {
        // this->connection_status_mtx.lock();
        if (this->connection_status != ConnectionStatus::CONNECTED && this->connection_status != ConnectionStatus::CONNECTING) {
            // this->connection_status_mtx.unlock();
            Log(LogLevel::WARNING, "[transaction] status is not connected. not listening to messages from server anymore.");
            
            int a = static_cast<int>(this->connection_status);

            Log(LogLevel::WARNING, "[transaction] status: " + std::to_string(a));

            break;
        } // exists once the connection is over
        // this->connection_status_mtx.unlock(); 

        // raw bytes
        auto data = this->client->receive_bytes();

        Log(LogLevel::INFO, "[transaction] received a package from server");

        // deserializing into SlowPackage
        auto package = SlowPackage::deserialize(data);

        this->buffer_mtx.lock();
        this->receiver_buffer.emplace_back(*package);
        this->buffer_mtx.unlock();
    }
}