#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <thread>
#include <unistd.h>
#include <vector>
#include "ClientNode.hpp"

ClientNode::ClientNode(std::string serverIP)
{
    this->client = Socket();
    this->client.connect(serverIP);

    std::thread t1(&ClientNode::handleServer, this);
    t1.detach();
}

void ClientNode::writeFile(std::string path, std::vector<char>& data)
{
    long msSinceUnixEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ofstream file( std::filesystem::path((path + "-" + std::to_string(msSinceUnixEpoch)).c_str()), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary );
    for (const auto& byte : data)
        file << byte;
}

void ClientNode::handleServer()
{
    while (true)
    {
        int n;
        unsigned int m = sizeof(n);
        int fdsocket;
        fdsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); // example
        getsockopt(fdsocket,SOL_SOCKET,SO_RCVBUF,(void *)&n, &m);
        // now the variable n will have the socket size
        std::cout << n << " is the socket size \n";

        std::cout << "Listening...\n";

        int filenameLength;
        this->client.recvall((char*)&filenameLength, sizeof(int));
        std::cout << "filenameLength: " << filenameLength << "\n";
        std::vector<char> filename(filenameLength);
        this->client.recvall(&filename[0], filenameLength);
        std::string stringFilename(filename.cbegin(), filename.cend());
        std::cout << "filename: " << stringFilename << "\n";
        int dataLength;
        this->client.recvall((char*)&dataLength, sizeof(int));
        std::cout << "dataLength: " << dataLength << "\n";
        std::vector<char> data(dataLength);
        this->client.recvall(&data[0], dataLength);
        std::cout << "data...\n";
        std::cout << "data size: " << data.size() << "\n";
        //std::cout << "Last elements: " << (data[128382] == (char)252 ? "passed" : "failed") << (data[128383] == (char)219 ? "passed" : "failed") << (data[128384] == 0 ? "passed" : "failed") << "\n";

        writeFile(std::filesystem::path(this->saveDirectory) / std::filesystem::path(stringFilename), data);
    }
}

ClientNode::~ClientNode()
{
    this->client.close();
}