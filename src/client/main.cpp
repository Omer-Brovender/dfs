#include <string>
#include "../common/Socket.hpp"

int main()
{
    Socket client;

    client.connect("127.0.0.1");

    std::string msg = "Hello, world!\n";
    client.send(msg.data(), msg.size());

    client.close();
}