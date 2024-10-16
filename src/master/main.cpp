#include <iostream>
#include "../common/Socket.hpp"
#include "MasterNode.hpp"

int main()
{
    MasterNode master;

    Socket server;

    server.bind();
    server.listen(1);
    int client = server.accept();
    char buffer[1024];

    server.recv(client, buffer, sizeof(buffer)/sizeof(char));
    std::string str;
    std::cout << std::string(&buffer[0]);

    server.close();
}