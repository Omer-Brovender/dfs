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
    std::string workingPathString;
    std::cout << "Working path: ";
    std::cin >> workingPathString;
    std::filesystem::path workingPath(workingPathString);
    std::shared_ptr<MasterNode> master = std::make_shared<MasterNode>(workingPath.string());
    std::cout << std::filesystem::current_path() << "\n";
    WebServer server((workingPath / "web").string(), (workingPath / "db.sqlite").string());
    server.setMasterNode(master);
    std::thread t([] (WebServer& server) 
    {
        server.start(443);
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