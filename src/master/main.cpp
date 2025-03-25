#include <cctype>
#include <chrono>
#include <iostream>
#include <thread>
#include "../common/Socket.hpp"
#include "MasterNode.hpp"
#include "WebServer.hpp"

int main()
{
#ifdef _WIN32
    static WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (wsaerr)
        exit(1);
#endif
    //MasterNode master("");
    WebServer server("", "");
    std::thread t([] (WebServer& server) 
    {
        server.start(8080);
    }, std::ref(server));


    while (true)
    {
        std::string in;
        //std::cout << "Upload/Download (U/D): ";
        //std::cin >> in;
        /*if (in.length() > 0)
        {*/
        //if (std::tolower(in[0]) == 'u')
        //{
        std::cout << "Upload File: ";
        std::cin >> in;
        if (in == "q") 
        {
            server.stop();
            break;
        };
        /////////master.upload(in);
        //}
            /*else if (std::tolower(in[0] == 'd'))
            {
                std::cout << "Enter filename (example.txt): ";
                std::cin >> in;
                master
            }
        }*/
    }
    return 0;
}