#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Database.hpp"
#include "MasterNode.hpp"
#include "crow/app.h"
#include "crow/middlewares/cookie_parser.h"
#include "crow/http_request.h"
#include "crow/http_response.h"
#include <functional>
#include <vector>

class WebServer
{
public:
    WebServer(std::string webRoot, std::string dbPath);
    void start(std::uint16_t port);
    void stop();
    void setMasterNode(std::shared_ptr<MasterNode> master);
    ~WebServer();

private:
    Database db;
    std::shared_ptr<MasterNode> master;

    crow::App<crow::CookieParser> app;
    std::string webRoot;
    crow::response rootPage(const crow::request& req);
    crow::response loginPage(const crow::request& req);
    crow::response login(const crow::request& req);
    crow::response signup(const crow::request& req);
    crow::response upload(const crow::request& req);
    crow::response files(const crow::request& req);
    crow::response download(const crow::request& req, int id);

    void setupRoutes();
};

#endif