#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "MasterNode.hpp"

MasterNode::MasterNode()
{
    this->server = Socket();
    this->server.bind();
    this->server.listen(5);

    std::thread t1(&MasterNode::acceptClients, this);
    t1.detach();

    this->chunkIndex = 0;
}

MasterNode::~MasterNode()
{
    this->server.close();
}

void MasterNode::acceptClients()
{
    while (true)
    {
        std::cout << "Listening for clients...\n";
        int client = this->server.accept();
        this->clientsMutex.lock();
        this->clients.push_back(client);
        this->clientsMutex.unlock();
    }
}

std::vector<char> MasterNode::readFile(std::string path)
{
    std::ifstream file( std::filesystem::path(path.c_str()), std::ifstream::in | std::ifstream::binary );
    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const auto size = std::filesystem::file_size(path);
    data.resize(size);

    return data;
}

std::vector<uint64_t> MasterNode::splitData(uint64_t dataSize, uint64_t chunkSize)
{
    std::vector<uint64_t> ranges;
    for (int c = 0; c < dataSize; c += chunkSize) ranges.push_back(c);
    ranges.push_back(dataSize);
    return ranges;
}

void MasterNode::uploadByProtocol(int client, std::string& filename, char* data, int dataLength)
{
    int filenameLength = filename.length();

    this->server.sendall(client, (char*)&filenameLength, sizeof(int));
    this->server.sendall(client, filename.data(), filenameLength);
    this->server.sendall(client, (char*)&dataLength, sizeof(int));
    this->server.sendall(client, data, dataLength);
}

void MasterNode::upload(std::string path)
{
    this->clientsMutex.lock();
    if (this->clients.size() == 0)
    {
        std::cout << "No clients are connected, cannot upload file!\n";
        return;
    }
    this->clientsMutex.unlock();

    int chunkSize = 50000;
    std::vector<char> data = readFile(path);
    int amountTransferred = 0;
    while (amountTransferred < data.size())
    {
        this->clientsMutex.lock();
        for (int client : this->clients)
        {
            unsigned long currChunkSize = std::min(data.size() - amountTransferred, (unsigned long)chunkSize);
            std::vector<char>::const_iterator first = data.begin() + amountTransferred;
            std::vector<char>::const_iterator last = data.begin() + amountTransferred + currChunkSize;
            std::vector<char> currChunk = std::vector(first, last);
            //this->server.send(client, currChunk.data(), currChunk.size());
            /*std::string filename;
            #ifdef __linux__
                filename = path.substr(path.find_last_of('/')+1);
            #elif _WIN32
                filename = path.substr(path.find_last_of('\\')+1);
            #endif*/
            std::string filename = ("C-" + std::to_string(this->chunkIndex++));
            uploadByProtocol(client, filename, &currChunk[0], currChunk.size());

            amountTransferred += currChunkSize;
            if (amountTransferred > data.size()) break;
        }
        this->clientsMutex.unlock();
    }
}