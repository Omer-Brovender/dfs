#ifndef CLIENTNODE_HPP
#define CLIENTNODE_HPP

#include "../common/Socket.hpp"

class ClientNode
{
private:
    Socket client;
    void writeFile(std::wstring path, std::vector<char> data);
    void handleServer();
public:
    ClientNode(std::string serverIP);
    ~ClientNode();

};

#endif