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

    std::this_thread::sleep_for(std::chrono::seconds(5));
    master.upload("...");
    getchar();
}