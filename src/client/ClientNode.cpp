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
        std::cout << "Listening...\n";

        int filenameLength;
        this->client.recv((char*)&filenameLength, sizeof(int));
        std::cout << "filenameLength: " << filenameLength << "\n";
        std::vector<char> filename(filenameLength);
        this->client.recv(&filename[0], filenameLength);
        std::string stringFilename(filename.cbegin(), filename.cend());
        std::cout << "filename: " << stringFilename << "\n";
        int dataLength;
        this->client.recv((char*)&dataLength, sizeof(int));
        std::cout << "dataLength: " << dataLength << "\n";
        std::vector<char> data(dataLength);
        this->client.recv(&data[0], dataLength);
        std::cout << "data...\n";

        writeFile(std::filesystem::path(this->saveDirectory) / std::filesystem::path(stringFilename), data);
    }
}

ClientNode::~ClientNode()
{
    this->client.close();
}