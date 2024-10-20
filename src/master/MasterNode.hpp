#ifndef MASTERNODE_HPP
#define MASTERNODE_HPP

#include <mutex>
#include <string>
#include <vector>
#include <stdint.h>
#include "../common/Socket.hpp"

class MasterNode
{
private:
    Socket server;
    std::vector<int> clients;
    std::mutex clientsMutex;
    int chunkIndex;
    void acceptClients();
    std::vector<char> readFile(std::string path);
    void uploadByProtocol(int client, std::string& filename, char* data, int dataLength);
    std::vector<uint64_t> splitData(uint64_t dataSize, uint64_t chunkSize);
public:
    void upload(std::string path);
    MasterNode();
    ~MasterNode();

};

#endif