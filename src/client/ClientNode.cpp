#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>
#include "ClientNode.hpp"

ClientNode::ClientNode(std::string serverIP)
{
    this->client.connect(serverIP);

    std::thread t1(&ClientNode::handleServer, this);
    t1.detach();
}

void ClientNode::writeFile(std::string path, std::vector<char>& data)
{
    std::ofstream file( std::filesystem::path(path), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary );
    for (const auto& byte : data)
        file << byte;
    file.close();
}

void ClientNode::handleUpload()
{
    std::string stringFilename;
    std::vector<char> data;
    this->client.downloadFile((int)NULL, &stringFilename, &data, false);
    writeFile(std::filesystem::path(this->saveDirectory) / std::filesystem::path(stringFilename), data);
}

void ClientNode::handleDownload()
{
    this->client.uploadFile(this->saveDirectory);
}

void ClientNode::handleServer()
{
    while (true)
    {
        std::cout << "Listening...\n";

        PacketType action;
        this->client.recvall((char*)&action, sizeof(PacketType));

        switch(action)
        {
            case PacketType::UPLOAD:
                handleUpload();
                break;
            case PacketType::DOWNLOAD:
                handleDownload();
                break;
            case PacketType::EXIT:
                this->client.close();
                exit(1);
                break; // For consistency
            default:
                std::cout << "Unknown packet type, cannot handle server request!\n"; 
        }  
    }
}

ClientNode::~ClientNode()
{
    this->client.close();
}