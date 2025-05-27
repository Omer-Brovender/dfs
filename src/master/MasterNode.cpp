#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif
#include <thread>
#include <vector>
#include "../common/Socket.hpp"
#include "MasterNode.hpp"

// Default content for the metadata JSON file
constexpr static char jsonDefaultData[] = "{ \"data\": {} }";

// Constructor: initializes the master node, binds to a port, starts accepting clients, and loads JSON save file
MasterNode::MasterNode(std::string saveDirectory)
    : saveDirectory(saveDirectory)
{
    // Bind socket and start listening for client connections
    this->server.bind();
    this->server.listen(5);

    // Start background thread to accept clients asynchronously
    std::thread t1(&MasterNode::acceptClients, this);
    t1.detach();

    // Path to JSON save file containing file-chunk metadata
    std::filesystem::path saveFilePath(std::filesystem::path(saveDirectory) / "dfs.json");

    // If file doesn't exist, create it with default empty structure
    if (!std::filesystem::exists(saveFilePath))
    {
        std::fstream saveFileOut(saveFilePath, std::fstream::out);
        saveFileOut << jsonDefaultData;
        saveFileOut.close();
    }

    // Load existing metadata from file into memory
    std::fstream saveFile(saveFilePath, std::fstream::in);
    std::cout << std::filesystem::file_size(saveFilePath) << "\n";
    saveFile >> this->save;
    saveFile.close();
}

// Destructor: notifies all clients to disconnect, saves metadata, and closes socket
MasterNode::~MasterNode()
{
    std::cout << "Exiting\n";

    // Notify all connected clients to exit
    this->clientsMutex.lock();
    for (auto& [ip, client] : this->clients)
    {
        PacketType action = PacketType::EXIT;
        this->server.send(client, (char*)&action, sizeof(action));
    }
    this->clientsMutex.unlock();

    // Save current state to disk
    writeSaveFile();

    // Close the server socket
    this->server.close();
}

// Accepts incoming client connections and stores them with their IP
void MasterNode::acceptClients()
{
    while (1)
    {
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        int client = this->server.accept((sockaddr*)&addr, &len);
        std::string ip = Socket::IPToString(addr, len);
        std::cout << "IP: " << ip << "\n";

        if (client == -1) continue;

        std::cout << "Accepted! client: " << client << "\n";

        // Add the new client to the map of connected clients
        this->clientsMutex.lock();
        this->clients[ip] = client;
        this->clientsMutex.unlock();
    }
}

// Downloads and reassembles all chunks of a file by its index
std::vector<char> MasterNode::downloadFile(int fileIndex)
{
    std::vector<char> fileData;

    // Read metadata: number of chunks and their host IPs
    int chunks = this->save["data"][std::to_string(fileIndex)]["chunks"];
    std::vector<std::string> hosts = this->save["data"][std::to_string(fileIndex)]["hosts"];

    // Request each chunk from the appropriate host and concatenate the data
    for (int i = 0; i < chunks; ++i)
    {
        std::string ip = hosts[i];
        std::vector<char> chunkData = download(ip, fileIndex, i);
        fileData.insert(fileData.cend(), chunkData.cbegin(), chunkData.cend());
    }

    return fileData;
}

// Downloads a specific chunk from a specific client IP
std::vector<char> MasterNode::download(std::string& ip, int fileIndex, int chunkIndex)
{
    std::vector<char> fileData;

    // Locate client connection by IP
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

    // Construct filename for the requested chunk
    std::string filename = "I-" + std::to_string(fileIndex) + "-C-" + std::to_string(chunkIndex);

    // Initiate download from the client
    this->server.downloadFile(client, &filename, &fileData, true);

    return fileData;
}

// Saves the current file-chunk metadata JSON to disk
void MasterNode::writeSaveFile()
{
    std::filesystem::path saveFilePath(std::filesystem::path(this->saveDirectory) / "dfs.json");
    std::ofstream saveFile(saveFilePath, std::ifstream::out);
    saveFile << this->save;
    saveFile.close();
}

// Uploads a file by splitting it into chunks and sending to connected clients
void MasterNode::upload(std::vector<char>& data, int id)
{
    // Ensure there are clients to upload to
    this->clientsMutex.lock();
    if (this->clients.size() == 0)
    {
        std::cout << "No clients are connected, cannot upload file!\n";
        return;
    }
    this->clientsMutex.unlock();

    int chunkSize = 50000; // Size of each chunk
    int amountTransferred = 0;

    // Prepare metadata containers
    json file = json::object();
    json chunkInfo = json::array();
    int chunks = 0;
    int chunkIndex = 0;

    // Continue until the entire file has been chunked and uploaded
    while (amountTransferred < data.size())
    {
        this->clientsMutex.lock();
        for (auto [ip, client] : this->clients)
        {
            chunks++;

            // Determine current chunk size
            unsigned long currChunkSize = (std::min)((unsigned long)(data.size() - amountTransferred), (unsigned long)chunkSize);

            // Extract the chunk from the main data vector
            std::vector<char>::const_iterator first = data.begin() + amountTransferred;
            std::vector<char>::const_iterator last = data.begin() + amountTransferred + currChunkSize;
            std::vector<char> currChunk = std::vector(first, last);

            // Generate unique filename for the chunk
            std::string filename = ("I-" + std::to_string(id) + "-C-" + std::to_string(chunkIndex++));

            // Upload the chunk to the client
            this->server.initiateUploadFile(client, filename, &currChunk[0], currChunk.size());

            // Track which client received this chunk
            chunkInfo.push_back(std::string(ip));

            // Update total amount transferred
            amountTransferred += currChunkSize;

            // Stop if we've uploaded all data
            if (amountTransferred > data.size()) break;
        }
        this->clientsMutex.unlock();
    }

    // Store metadata for this file
    file["chunks"] = chunks;
    file["hosts"] = chunkInfo;
    this->save["data"][std::to_string(id)] = file;

    // Save metadata to disk
    writeSaveFile();
}
