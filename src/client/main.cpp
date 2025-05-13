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

    std::string IP;
    std::cout << "Target IP: ";
    std::cin >> IP;

    ClientNode client(IP);
    std::cout << "Path: ";
    std::cin >> client.saveDirectory;
    // client.saveDirectory = "C:\\Users\\omerb\\Documents\\testing\\";
    getchar();
}