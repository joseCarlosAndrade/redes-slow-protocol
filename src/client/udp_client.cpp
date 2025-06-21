#include "udp_client.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> 
#include <cstring>

UdpClient::UdpClient(const std::string& host, int port)
    : host(host), port(port), sockfd(-1), is_connected(false) {
    // Inicializa a estrutura de endereço do servidor com zeros
    memset(&servaddr, 0, sizeof(servaddr));
}

UdpClient::~UdpClient() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

bool UdpClient::setupConnection() {
    // 1. Criar o socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Falha ao criar socket");
        return false;
    }

    // 2. Resolver o nome do host
    struct hostent *server = gethostbyname(host.c_str());
    if (server == nullptr) {
        std::cerr << "Erro: Host nao encontrado: " << host << std::endl;
        return false;
    }

    // 3. Preencher informações do servidor
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port); 
    memcpy(&servaddr.sin_addr, server->h_addr_list[0], server->h_length);
    
    is_connected = true;
    std::cout << "Conexao UDP configurada para " << host << ":" << port << std::endl;
    return true;
}

bool UdpClient::setReceiveTimeout(long seconds, long microseconds) {
    if (!is_connected) {
        std::cerr << "Erro: Socket nao esta conectado para definir timeout." << std::endl;
        return false;
    }

    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = microseconds;
    
    // SO_RCVTIMEO define o timeout de recebimento
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Erro ao definir timeout de recebimento");
        return false;
    }
    return true;
}


bool UdpClient::send_chars(const std::vector<char>& data) {
    if (!is_connected) {
        std::cerr << "Erro: Socket nao conectado." << std::endl;
        return false;
    }

    // sendto envia os dados para o endereço de servidor configurado
    ssize_t bytes_sent = sendto(sockfd, data.data(), data.size(), 0, 
                                (const struct sockaddr *)&servaddr, sizeof(servaddr));

    if (bytes_sent < 0) {
        perror("Falha no envio de dados");
        return false;
    }

    return true;
}

bool UdpClient::send_bytes(const std::vector<std::byte>& data) {
    if (!is_connected) {
        std::cerr << "Erro: Socket nao conectado." << std::endl;
        return false;
    }

    // sendto envia os dados para o endereço de servidor configurado
    ssize_t bytes_sent = sendto(sockfd, data.data(), data.size(), 0, 
                                (const struct sockaddr *)&servaddr, sizeof(servaddr));

    if (bytes_sent < 0) {
        perror("Falha no envio de dados");
        return false;
    }

    return true;
}

std::vector<char> UdpClient::receive_chars(int buffer_size) {
    if (!is_connected) {
        std::cerr << "Erro: Socket nao conectado." << std::endl;
        return {};
    }

    std::vector<char> buffer(buffer_size);
    socklen_t len = sizeof(servaddr);

    // recvfrom aguarda por dados
    ssize_t bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), 0, 
                                      (struct sockaddr *)&servaddr, &len);

    if (bytes_received < 0) {
        perror("Falha no recebimento de dados (ou timeout)");
        return {};
    }

    // Redimensiona o buffer para o tamanho real de dados recebidos
    buffer.resize(bytes_received);
    return buffer;
}


std::vector<std::byte> UdpClient::receive_bytes(int buffer_size) {
        if (!is_connected) {
        std::cerr << "Erro: Socket nao conectado." << std::endl;
        return {};
    }

    std::vector<std::byte> buffer(buffer_size);
    socklen_t len = sizeof(servaddr);

    // recvfrom aguarda por dados
    ssize_t bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), 0, 
                                      (struct sockaddr *)&servaddr, &len);

    if (bytes_received < 0) {
        perror("Falha no recebimento de dados (ou timeout)");
        return {};
    }

    // Redimensiona o buffer para o tamanho real de dados recebidos
    buffer.resize(bytes_received);
    return buffer;
}