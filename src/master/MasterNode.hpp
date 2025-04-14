#ifndef MASTERNODE_HPP
#define MASTERNODE_HPP

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdint.h>
#include <jsonlib/single_include/nlohmann/json.hpp>
#include "../common/Socket.hpp"

using nlohmann::json;

class MasterNode
{
public:
    std::string saveDirectory;

    std::vector<char> downloadFile(int fileIndex);
    std::vector<char> download(std::string& ip, int fileIndex, int chunkIndex);
    void upload(std::vector<char>& data, int id);
    //void upload(std::string path);
    std::vector<int> getFiles(std::string query);
    MasterNode(std::string saveDirectory);
    ~MasterNode();
    
private:
    Socket server;
    std::unordered_map<std::string, int> clients;
    std::mutex clientsMutex;
    int fileIndex;
    json save;

    void acceptClients();
    void writeSaveFile();
    void uploadByProtocol(int client, std::string& filename, char* data, int dataLength);
};

#endif