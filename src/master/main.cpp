#include <cctype>
#include <chrono>
#include <iostream>
#include <memory>
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
    std::shared_ptr<MasterNode> master = std::make_shared<MasterNode>("");
    WebServer server("", "");
    server.setMasterNode(master);
    std::thread t([] (WebServer& server) 
    {
        server.start(8080);
    }, std::ref(server));


    while (true)
    {
        std::string in;
        std::cout << "Press q to quit: ";
        std::cin >> in;
        if (in.length() > 0)
        {
            if (in == "q") 
            {
                server.stop();
                break;
            };

            /*if (std::tolower(in[0]) == 'u')
            {
                std::cout << "Upload File: ";
                std::cin >> in;

                master->upload(in);
            }
            else if (std::tolower(in[0] == 'd'))
            {
                std::cout << "Enter filename (example.txt): ";
                std::cin >> in;
                //master
            }*/
        }
    }
    return 0;
}