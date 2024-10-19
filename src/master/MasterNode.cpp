#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
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

std::vector<char> MasterNode::readFile(std::wstring path)
{
    std::ifstream file( std::filesystem::path(path.c_str()) );
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

    this->server.send(client, (char*)&filenameLength, sizeof(int));
    this->server.send(client, filename.data(), filenameLength);
    this->server.send(client, (char*)&dataLength, sizeof(int));
    this->server.send(client, data, dataLength);
}

void MasterNode::upload(std::wstring path)
{
    this->clientsMutex.lock();
    if (this->clients.size() == 0)
    {
        std::cout << "No clients are connected, cannot upload file!\n";
        return;
    }
    this->clientsMutex.unlock();

    int chunkSize = 2e6;
    std::vector<char> data = readFile(path);
    int amountTransferred = 0;
    while (amountTransferred < data.size())
    {
        this->clientsMutex.lock();
        for (int client : this->clients)
        {
            unsigned long currChunkSize = std::min(data.size(), (unsigned long)chunkSize);
            std::vector<char>::const_iterator first = data.begin() + amountTransferred;
            std::vector<char>::const_iterator last = data.begin() + amountTransferred + currChunkSize;
            std::vector<char> currChunk = std::vector(first, last);
            //this->server.send(client, currChunk.data(), currChunk.size());
            std::string filename = "Testing";
            uploadByProtocol(client, filename, currChunk.data(), currChunk.size());

            amountTransferred += currChunkSize + 1; // To avoid duplicating the last byte
            std::cout << amountTransferred << "\n";
            if (amountTransferred > data.size()) break;
        }
        this->clientsMutex.unlock();
    }
}