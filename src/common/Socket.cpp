#include "Socket.hpp"

#include <cstddef>
#include <cstring>
#include <errno.h>
#include <vector>

#ifdef __linux__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
#include <WinSock2.h>
#endif

#include <iostream>

Socket::Socket()
{
    this->fileDesc = socket(AF_INET, SOCK_STREAM, 0);

    if (this->fileDesc == -1)
    {
        std::cout << "Error: could not initialize socket. Error number: " << errno << "\n";
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12347);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    this->serverAddress = serverAddress;
}

void Socket::bind()
{
    if (::bind(this->fileDesc, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1) 
        std::cout << "Failed to bind. Error number: " << errno << "\n";
}

void Socket::listen(int maxListeners)
{
    if (::listen(this->fileDesc, maxListeners) == -1)
        std::cout << "Failed to listen. Error number: " << errno << "\n";
}

void Socket::connect(std::string IP)
{
    this->serverAddress.sin_addr.s_addr = inet_addr(IP.c_str());
    if (::connect(this->fileDesc, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1)
        std::cout << "Failed to connect. Error number: " << errno << "\n";
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
    ) == -1) std::cout << "Failed to send. Error number: " << errno << "\n";
}

void Socket::send(int fd, char* data, size_t size)
{
    int res = ::send(fd, data, size, 0);
    if (res == -1) std::cout << "Failed to send. Error number: " << errno << "\n";
}

void Socket::sendall(int fd, char* data, size_t size)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = size; // how many we have left to send
    int n;

    while(total < size) {
        n = ::send(fd, data+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
}

void Socket::sendall(char* data, size_t size)
{
    int total = 0;        
    int bytesleft = size; 
    int n;

    while(total < size) {
        n = ::send(this->fileDesc, data+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
}

void Socket::recvall(int fd, char* buffer, size_t size)
{
    int res = ::recv(fd, buffer, size, MSG_WAITALL); 
    if (res == -1) std::cout << "Failed to receive. Error number: " << errno << "\n";
}

void Socket::recvall(char* buffer, size_t size)
{
    int res = ::recv(this->fileDesc, buffer, size, MSG_WAITALL); 
    if (res == -1) std::cout << "Failed to receive. Error number: " << errno << "\n";
}

void Socket::recv(char* buffer, size_t size)
{
    int res = ::recv(this->fileDesc, buffer, size, 0); 
    if (res == -1) std::cout << "Failed to receive. Error number: " << errno << "\n";
}

void Socket::recv(int fd, char* buffer, size_t size)
{
    if (::recv
    (
        fd, 
        buffer, 
        size,
        0
    ) == -1) std::cout << "Failed to receive. Error number: " << errno << "\n";
}

void Socket::downloadFile(std::string* outFilename, std::vector<char>* outData)
{
    int filenameLength;
    recvall((char*)&filenameLength, sizeof(int));
    
    std::vector<char> filename(filenameLength);
    recvall(&filename[0], filenameLength);
    std::string stringFilename(filename.cbegin(), filename.cend());
    
    int dataLength;
    recvall((char*)&dataLength, sizeof(int));
    
    std::vector<char> data(dataLength);
    recvall(&data[0], dataLength);

    std::cout << "filenameLength: " << filenameLength << "\n";
    std::cout << "filename: " << stringFilename << "\n";
    std::cout << "dataLength: " << dataLength << "\n";
    std::cout << "data...\n";

    *outFilename = stringFilename;
    *outData = data;
}

void Socket::uploadFile(int client, std::string& filename, char* data, int dataLength)
{
    int filenameLength = filename.length();
    PacketType action = PacketType::UPLOAD;

    sendall(client, (char*)&action, sizeof(PacketType));
    sendall(client, (char*)&filenameLength, sizeof(int));
    sendall(client, filename.data(), filenameLength);
    sendall(client, (char*)&dataLength, sizeof(int));
    sendall(client, data, dataLength);
}
void Socket::close()
{
#ifdef __linux__
    ::close(this->fileDesc);
#elif _WIN32
    ::closesocket(this->fileDesc);
#endif
}