#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <cstddef>
#include <cstring>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

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

    void recv(char* buffer, size_t size);
    void recv(int fd, char* buffer, size_t size);

    void close();

};

#endif