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
    file.close();
}

void ClientNode::handleUpload()
{
    int filenameLength;
    this->client.recvall((char*)&filenameLength, sizeof(int));
    
    std::vector<char> filename(filenameLength);
    this->client.recvall(&filename[0], filenameLength);
    std::string stringFilename(filename.cbegin(), filename.cend());
    
    int dataLength;
    this->client.recvall((char*)&dataLength, sizeof(int));
    
    std::vector<char> data(dataLength);
    this->client.recvall(&data[0], dataLength);

    std::cout << "filenameLength: " << filenameLength << "\n";
    std::cout << "filename: " << stringFilename << "\n";
    std::cout << "dataLength: " << dataLength << "\n";
    std::cout << "data...\n";

    writeFile(std::filesystem::path(this->saveDirectory) / std::filesystem::path(stringFilename), data);
}

void ClientNode::handleServer()
{
    while (true)
    {
        std::cout << "Listening...\n";

        PacketType action;
        this->client.recvall((char*)&action, sizeof(PacketType));

        if (action == PacketType::UPLOAD) 
        {
            handleUpload();
        } else {
            std::cout << "Unknown packet type, cannot handle server request!\n";    
        }    
    }
}

ClientNode::~ClientNode()
{
    this->client.close();
}