#include <string>
#include "../common/Socket.hpp"

int main()
{
#ifdef _WIN32
    static WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (wsaerr)
        exit(1);
#endif

    Socket client;

    client.connect("127.0.0.1");

    std::string msg = "Hello, world!\n";
    client.send(msg.data(), msg.size());

    client.close();
    getchar();
}