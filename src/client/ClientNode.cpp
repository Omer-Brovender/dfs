#include <fstream>
#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>
#include "ClientNode.hpp"

ClientNode::ClientNode(std::string serverIP)
{
    this->client = Socket();
    this->client.connect(serverIP);

    std::thread t1(&ClientNode::handleServer, this);
    t1.detach();
}

void ClientNode::writeFile(std::wstring path, std::vector<char> data)
{
    std::ofstream file( std::filesystem::path(path.c_str()), std::ofstream::out | std::ofstream::trunc );
    file << data.data();
}

void ClientNode::handleServer()
{
    while (true)
    {
        std::cout << "Listening...\n";

        int filenameLength;
        this->client.recv((char*)&filenameLength, sizeof(int));
        
        std::vector<char> filename(filenameLength);
        this->client.recv(&filename[0], filenameLength);
        std::string stringFilename(filename.cbegin(), filename.cend());
        
        int dataLength;
        this->client.recv((char*)&dataLength, sizeof(int));
        
        std::vector<char> data(dataLength);
        this->client.recv(&data[0], dataLength);

        std::cout << "filenameLength: " << filenameLength << "\n";
        std::cout << "filename: " << stringFilename << "\n";
        std::cout << "dataLength: " << dataLength << "\n";
        std::cout << "data: " << std::string(data.data()) << "\n";
    }
}

ClientNode::~ClientNode()
{
    this->client.close();
}