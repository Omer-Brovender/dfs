#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <cstddef>
#include <cstring>
#include <vector>
#include <filesystem>
#ifdef __linux__
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
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
public:
    Socket();

    static std::string IPToString(struct sockaddr_storage& addr, socklen_t& len);

    void connect(std::string IP);
    void bind();
    void listen(int maxListeners);
    int accept();
    int accept(struct sockaddr* addr, socklen_t *len);
    
    void send(char* data, size_t size);
    void send(int fd, char* data, size_t size);

    void sendall(char* data, size_t size);
    void sendall(int fd, char* data, size_t size);

    void recv(char* buffer, size_t size);
    void recv(int fd, char* buffer, size_t size);

    void recvall(char* buffer, size_t size);
    void recvall(int fd, char* buffer, size_t size);

    void uploadFile(std::filesystem::path dir);
    void initiateUploadFile(int target, std::string& filename, char* data, int dataLength);
    void downloadFile(int optionalTarget, std::string* outFilename, std::vector<char>* outData, bool initiating);

    void close();
    ~Socket();

private:
    int fileDesc;
    std::vector<int> clientFileDesc;
    struct sockaddr_in serverAddress;
};

#endif