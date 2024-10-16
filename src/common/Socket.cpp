#include "Socket.hpp"

#include <cstddef>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

Socket::Socket()
{
    this->fileDesc = socket(AF_INET, SOCK_STREAM, 0);

    if (this->fileDesc == -1)
    {
        std::cout << "Error: could not initialize socket.\n";
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    this->serverAddress = serverAddress;
}

void Socket::bind()
{
    if (::bind(this->fileDesc, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1) std::cout << "ERROR\n";
}

void Socket::listen(int maxListeners)
{
    ::listen(this->fileDesc, maxListeners);
}

void Socket::connect(std::string IP)
{
    this->serverAddress.sin_addr.s_addr = inet_addr(IP.c_str());
    if (::connect(this->fileDesc, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1) std::cout << "ERROR\n";
}

int Socket::accept()
{
    return ::accept(this->fileDesc, nullptr, nullptr);
}

void Socket::send(char* data, size_t size)
{
    if (::send
    (
        this->fileDesc, 
        data, 
        size, 
        0
    ) == -1) std::cout << "ERROR\n";
}

void Socket::send(int fd, char* data, size_t size)
{
    if (::send
    (
        fd, 
        data, 
        size, 
        0
    ) == -1) std::cout << "ERROR\n";
}

void Socket::recv(char* buffer, size_t size)
{
    if (::recv
    (
        this->fileDesc, 
        buffer, 
        size,
        0
    ) == -1) std::cout << "ERROR\n";
}

void Socket::recv(int fd, char* buffer, size_t size)
{
    if (::recv
    (
        fd, 
        buffer, 
        size,
        0
    ) == -1) std::cout << "ERROR\n";
}

void Socket::close()
{
    ::close(this->fileDesc);
}