#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include "../common/Socket.hpp"
#include "MasterNode.hpp"

constexpr static char jsonDefaultData[] = "{ \"data\": {} }";

MasterNode::MasterNode(std::string saveDirectory)
: saveDirectory(saveDirectory)
{
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
}

MasterNode::~MasterNode()
{
    std::cout << "Exiting\n";
    this->clientsMutex.lock();
    for (auto& [ip, client] : this->clients)
    {
        PacketType action = PacketType::EXIT;
        this->server.send(client, (char*)&action, sizeof(action));
    }
    this->clientsMutex.unlock();

    writeSaveFile();
    this->server.close();
}

void MasterNode::acceptClients()
{
    while (1)
    {
        //std::cout << "Listening for clients...\n";
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        int client = this->server.accept((sockaddr*)&addr, &len);
        std::string ip = Socket::IPToString(addr, len);
        std::cout << "IP: " << ip << "\n";
        if (client == -1) continue;
        
        std::cout << "Accepted! client: " << client << "\n";
        this->clientsMutex.lock();
        this->clients[ip] = client;
        this->clientsMutex.unlock();
    }
}

std::vector<char> MasterNode::downloadFile(int fileIndex)
{
    std::vector<char> fileData;
    int chunks = this->save["data"][std::to_string(fileIndex)]["chunks"];
    std::vector<std::string> hosts = this->save["data"][std::to_string(fileIndex)]["hosts"];

    for (int i = 0; i < chunks; ++i)
    {
        std::string ip = hosts[i];
        std::vector<char> chunkData = download(ip, fileIndex, i);
        fileData.insert(fileData.cend(), chunkData.cbegin(), chunkData.cend());
    }

    return fileData;
}

std::vector<char> MasterNode::download(std::string& ip, int fileIndex, int chunkIndex)
{
    std::vector<char> fileData;

    this->clientsMutex.lock();
    auto clientIter = this->clients.find(ip);
    if (clientIter == this->clients.end())
    {
        std::cout << "Couldn't download FileI " << fileIndex << " ChunkI " << chunkIndex << " from " << ip << "!";
        this->clientsMutex.unlock();
        return fileData;
    }
    this->clientsMutex.unlock();

    int client = clientIter->second;
    std::string filename = "I-" + std::to_string(fileIndex) + "-C-" + std::to_string(chunkIndex);
    this->server.downloadFile(client, &filename, &fileData, true);

    return fileData;
}

void MasterNode::writeSaveFile()
{
    std::filesystem::path saveFilePath(std::filesystem::path(this->saveDirectory) / "dfs.json");
    std::ofstream saveFile(saveFilePath, std::ifstream::out);
    saveFile << this->save;
    saveFile.close();
}

void MasterNode::upload(std::vector<char>& data, int id)
{
    this->clientsMutex.lock();
    if (this->clients.size() == 0)
    {
        std::cout << "No clients are connected, cannot upload file!\n";
        return;
    }
    this->clientsMutex.unlock();

    int chunkSize = 50000;
    int amountTransferred = 0;

    json file = json::object();
    json chunkInfo = json::array();
    int chunks = 0;
    int chunkIndex = 0;
    while (amountTransferred < data.size())
    {
        this->clientsMutex.lock();
        for (auto [ip, client] : this->clients)
        {
            chunks++;

            unsigned long currChunkSize = (std::min)((unsigned long)(data.size() - amountTransferred), (unsigned long)chunkSize);
            std::vector<char>::const_iterator first = data.begin() + amountTransferred;
            std::vector<char>::const_iterator last = data.begin() + amountTransferred + currChunkSize;
            std::vector<char> currChunk = std::vector(first, last);
            
            std::string filename = ("I-" + std::to_string(id) + "-C-" + std::to_string(chunkIndex++));
            this->server.initiateUploadFile(client, filename, &currChunk[0], currChunk.size());
            chunkInfo.push_back(std::string(ip));

            amountTransferred += currChunkSize;
            if (amountTransferred > data.size()) break;
        }
        this->clientsMutex.unlock();
    }
    file["chunks"] = chunks;
    file["hosts"] = chunkInfo;
    this->save["data"][std::to_string(id)] = file;
    writeSaveFile();
}

/*void MasterNode::upload(std::string path)
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

    std::vector<char> data = readFile(path);
    upload(data, filename); // Call the other (overloaded) "upload" method
}*/