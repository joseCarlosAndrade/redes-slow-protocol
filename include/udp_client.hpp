#pragma once

#include <string>
#include <vector>
#include <netinet/in.h>


class UdpClient {
public:
    
    UdpClient(const std::string& host, int port);

    ~UdpClient();

    bool setupConnection();

    bool send_chars(const std::vector<char>& data);

    bool send_bytes(const std::vector<std::byte>& data);

    std::vector<char> receive_chars(int buffer_size = 1472);

    std::vector<std::byte> receive_bytes(int buffer_size = 1472);

    bool setReceiveTimeout(long seconds, long microseconds);

private:
    std::string host;
    int port;
    int sockfd;
    struct sockaddr_in listening_address;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    bool is_connected;
};