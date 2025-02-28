#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "crow/app.h"
#include "crow/http_request.h"
#include "crow/http_response.h"

class WebServer
{
public:
    WebServer(std::string webRoot);
    void start(std::uint16_t port);
    void stop();
    ~WebServer();

private:
    crow::SimpleApp app;
    std::string webRoot;
    crow::response rootPage();
    crow::response loginPage();
    crow::response login(const crow::request& req);
    crow::response signup(const crow::request& req);

    void setupRoutes();
};

#endif