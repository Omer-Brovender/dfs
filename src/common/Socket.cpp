#include "Socket.hpp"
#include "FileUtils.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <errno.h>
#include <vector>
#include <iostream>

#ifdef __linux__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
#include <WinSock2.h>
#endif

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

std::string Socket::IPToString(struct sockaddr_storage& addr, socklen_t& len)
{
    int port;
    char ip[INET6_ADDRSTRLEN];

    //deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET) 
    {
        struct sockaddr_in *s = (struct sockaddr_in*)&addr;
        std::cout << "sin_addr: " << &s->sin_addr << "\n";
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
    } 
    else 
    { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&addr;
        std::cout << "sin6_addr: " << &s->sin6_addr << "\n";
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
    }

    return ip;
}

void Socket::bind()
{
    //int f = socket(AF_INET, SOCK_STREAM, 0);
    //std::cout << f << "(1)\n";
    std::cout << this->fileDesc << "(2)\n";
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

int Socket::accept(struct sockaddr* addr, socklen_t *len)
{
    return ::accept(this->fileDesc, addr, len);
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

void Socket::downloadFile(int target, std::string* outFilename, std::vector<char>* outData, bool initiating)
{
    if (initiating)
    {
        PacketType action = PacketType::DOWNLOAD;
        sendall(target, (char*)&action, sizeof(PacketType));

        std::cout << "fname: " << *outFilename << "\n";

        int filenameLength = outFilename->length();
        std::cout << "fnamelen: " << filenameLength << "\n";
        sendall(target, (char*)&filenameLength, sizeof(int));
        sendall(target, &outFilename->at(0), filenameLength);

        int dataLength;
        recvall(target, (char*)&dataLength, sizeof(int));
        
        std::vector<char> data(dataLength);
        recvall(target, &data[0], dataLength);

        std::cout << "dataLength: " << dataLength << "\n";
        std::cout << "data...\n";

        *outData = data;
    }
    else 
    {        
        int filenameLength;
        recvall((char*)&filenameLength, sizeof(int));
        
        std::vector<char> filename(filenameLength);
        recvall(&filename[0], filenameLength);
        std::string stringFilename(filename.cbegin(), filename.cend());

        *outFilename = stringFilename;

        std::cout << "filenameLength: " << filenameLength << "\n";
        std::cout << "filename: " << stringFilename << "\n";
            
        int dataLength;
        recvall((char*)&dataLength, sizeof(int));
        
        std::vector<char> data(dataLength);
        recvall(&data[0], dataLength);

        std::cout << "dataLength: " << dataLength << "\n";
        std::cout << "data...\n";

        *outData = data;
    }
}

void Socket::initiateUploadFile(int client, std::string& filename, char* data, int dataLength)
{
    int filenameLength = filename.length();
    PacketType action = PacketType::UPLOAD;

    sendall(client, (char*)&action, sizeof(PacketType));
    sendall(client, (char*)&filenameLength, sizeof(int));
    sendall(client, &filename[0], filenameLength);
    sendall(client, (char*)&dataLength, sizeof(int));
    sendall(client, data, dataLength);
}

void Socket::uploadFile(std::filesystem::path dir)
{
    int filenameLength;
    recvall((char*)&filenameLength, sizeof(int));
    std::cout << "fnamelen:" << filenameLength << "\n";

    std::vector<char> filename(filenameLength);
    recvall(&filename[0], filenameLength);
    std::string stringFilename(filename.cbegin(), filename.cend());
    std::cout << "fname:" << stringFilename << "\n";

    std::cout << "F: " << dir << " " << stringFilename << "\n";
    std::vector<char> data = FileUtils::readFile((dir / stringFilename).string());
    int dataLength = data.size();

    sendall((char*)&dataLength, sizeof(int));
    sendall(&data[0], dataLength);
}

void Socket::close()
{
#ifdef __linux__
    ::close(this->fileDesc);
#elif _WIN32
    ::closesocket(this->fileDesc);
#endif
}

Socket::~Socket()
{
    std::cout << "Closing\n";
    close();
}