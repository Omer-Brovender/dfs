#include <iostream>
#include "../common/Socket.hpp"
#include "MasterNode.hpp"

int main()
{
#ifdef _WIN32
    static WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (wsaerr)
        exit(1);
#endif
    MasterNode master;

    Socket server;

    server.bind();
    server.listen(1);
    int client = server.accept();
    char buffer[1024] = { 0 };

    server.recv(client, buffer, sizeof(buffer)/sizeof(char));
    std::string str;
    std::cout << std::string(&buffer[0]);

    server.close();
    getchar();
}