#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <cstddef>
#include <cstring>
#include <vector>
#ifdef __linux__
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
#include <WinSock2.h>
#endif
#include <string>

enum class PacketType
{
    UPLOAD,
    DOWNLOAD,
    EXIT
};

class Socket
{
private:
    int fileDesc;
    std::vector<int> clientFileDesc;
    struct sockaddr_in serverAddress;

public:
    Socket();
    void connect(std::string IP);
    void bind();
    void listen(int maxListeners);
    int accept();
    
    void send(char* data, size_t size);
    void send(int fd, char* data, size_t size);

    void sendall(char* data, size_t size);
    void sendall(int fd, char* data, size_t size);

    void recv(char* buffer, size_t size);
    void recv(int fd, char* buffer, size_t size);

    void recvall(char* buffer, size_t size);
    void recvall(int fd, char* buffer, size_t size);

    void uploadFile(int client, std::string& filename, char* data, int dataLength);
    void downloadFile(std::string* outFilename, std::vector<char>* outData);

    void close();

};

#endif