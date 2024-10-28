#ifndef MASTERNODE_HPP
#define MASTERNODE_HPP

#include <mutex>
#include <string>
#include <vector>
#include <stdint.h>
#include <jsonlib/single_include/nlohmann/json.hpp>
#include "../common/Socket.hpp"

using nlohmann::json;

class MasterNode
{
private:
    Socket server;
    std::vector<int> clients;
    std::mutex clientsMutex;
    int fileIndex;
    json save;

    void acceptClients();
    std::vector<char> readFile(std::string path);
    void writeSaveFile();
    void uploadByProtocol(int client, std::string& filename, char* data, int dataLength);

public:
    std::string saveDirectory;

    void upload(std::string path);
    std::vector<int> getFiles(std::string query);
    MasterNode(std::string saveDirectory);
    ~MasterNode();
};

#endif