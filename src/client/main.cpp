#include <string>
#include "../common/Socket.hpp"

int main()
{
    Socket client;

    client.connect();

    std::string msg = "Hello, world!\n";
    client.send(msg.data(), msg.size());
}