#include <chrono>
#include <iostream>
#include <thread>
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
    MasterNode master("...");

    while (true)
    {
        std::string in;
        std::cout << "Upload File: ";
        std::cin >> in;
        master.upload(in);
    }
    getchar();
}