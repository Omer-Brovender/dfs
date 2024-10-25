#ifndef CLIENTNODE_HPP
#define CLIENTNODE_HPP

#include "../common/Socket.hpp"

class ClientNode
{
private:
    Socket client;
    void writeFile(std::string path, std::vector<char>& data);
    void handleServer();
    void handleUpload();
public:
    std::string saveDirectory;
    ClientNode(std::string serverIP);
    ~ClientNode();

};

#endif