#include <iostream>
#include <string>
#include "../common/Socket.hpp"
#include "ClientNode.hpp"

int main()
{
#ifdef _WIN32
    static WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (wsaerr)
        exit(1);
#endif

    ClientNode client("127.0.0.1");
    client.saveDirectory = "";
    getchar();
}