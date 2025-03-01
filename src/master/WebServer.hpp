#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Database.hpp"
#include "crow/app.h"
#include "crow/http_request.h"
#include "crow/http_response.h"

class WebServer
{
public:
    WebServer(std::string webRoot, std::string dbPath);
    void start(std::uint16_t port);
    void stop();
    ~WebServer();

private:
    Database db;

    crow::SimpleApp app;
    std::string webRoot;
    crow::response rootPage();
    crow::response loginPage();
    crow::response login(const crow::request& req);
    crow::response signup(const crow::request& req);

    void setupRoutes();
};

#endif