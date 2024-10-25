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

constexpr static char jsonDefaultData[] = "{ \"availableIndex\": 0, \"data\": [] }";

MasterNode::MasterNode(std::string saveDirectory)
: saveDirectory(saveDirectory)
{
    this->server = Socket();
    this->server.bind();
    this->server.listen(5);

    std::thread t1(&MasterNode::acceptClients, this);
    t1.detach();

    std::filesystem::path saveFilePath(std::filesystem::path(saveDirectory) / "dfs.json");

    if (!std::filesystem::exists(saveFilePath))
    {
        std::fstream saveFileOut(saveFilePath, std::fstream::out); // Must only use 'out' to create file if doesn't exist
        saveFileOut << jsonDefaultData;
        saveFileOut.close();
    }
    
    std::fstream saveFile(saveFilePath, std::fstream::in);
    std::cout << std::filesystem::file_size(saveFilePath) << "\n";
    saveFile >> this->save;
    saveFile.close();

    this->fileIndex = this->save["availableIndex"];
    std::cout << this->fileIndex << " a\n";
}

MasterNode::~MasterNode()
{
    PacketType action = PacketType::EXIT;
    this->server.send((char*)&action, sizeof(action));
    writeSaveFile();
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
    file.close();

    return data;
}

void MasterNode::writeSaveFile()
{
    this->save["availableIndex"] = this->fileIndex;
    std::filesystem::path saveFilePath(std::filesystem::path(this->saveDirectory) / "dfs.json");
    std::ofstream saveFile(saveFilePath, std::ifstream::out);
    saveFile << this->save;
    saveFile.close();
}

std::vector<uint64_t> MasterNode::splitData(uint64_t dataSize, uint64_t chunkSize)
{
    std::vector<uint64_t> ranges;
    for (int c = 0; c < dataSize; c += chunkSize) ranges.push_back(c);
    ranges.push_back(dataSize);
    return ranges;
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

    std::string filename;
    #ifdef __linux__
        filename = path.substr(path.find_last_of('/')+1);
    #elif _WIN32
        filename = path.substr(path.find_last_of('\\')+1);
    #endif

    int chunkSize = 50000;
    std::vector<char> data = readFile(path);
    int amountTransferred = 0;

    json file = json::object();
    file["fileIndex"] = this->fileIndex;
    file["filename"] = filename;
    int chunks = 0;
    int chunkIndex = 0;
    while (amountTransferred < data.size())
    {
        this->clientsMutex.lock();
        for (int client : this->clients)
        {
            chunks++;

            unsigned long currChunkSize = std::min(data.size() - amountTransferred, (unsigned long)chunkSize);
            std::vector<char>::const_iterator first = data.begin() + amountTransferred;
            std::vector<char>::const_iterator last = data.begin() + amountTransferred + currChunkSize;
            std::vector<char> currChunk = std::vector(first, last);
            
            std::string filename = ("I-" + std::to_string(this->fileIndex) + "-C-" + std::to_string(chunkIndex++));
            this->server.uploadFile(client, filename, &currChunk[0], currChunk.size());

            amountTransferred += currChunkSize;
            if (amountTransferred > data.size()) break;
        }
        this->clientsMutex.unlock();
    }
    this->fileIndex++;
    file["chunks"] = chunks;
    this->save["data"].push_back(file);
    writeSaveFile();
}