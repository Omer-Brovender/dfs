#include "WebServer.hpp"
#include "Database.hpp"
#include "crow/app.h"
#include "crow/http_request.h"
#include "crow/http_response.h"
#include "crow/json.h"
#include "crow/mustache.h"

WebServer::WebServer(std::string webRoot, std::string dbPath)
: db(dbPath.c_str())
{
    crow::mustache::set_global_base(webRoot);
    setupRoutes();
}

void WebServer::start(std::uint16_t port)
{
    this->app.port(port)
        .multithreaded()
        .run();
}

void WebServer::stop()
{
    this->db.close();
    this->app.stop();
}

crow::response WebServer::rootPage()
{
    crow::mustache::context ctx;
    return crow::mustache::load("templates/index.html").render();
}

crow::response WebServer::loginPage()
{
    crow::mustache::context ctx;
    return crow::mustache::load("templates/login.html").render();
}

crow::response WebServer::login(const crow::request& req)
{
    auto data = crow::json::load(req.body);
    if (!data || !(data.has("email") && data.has("password")))
        return crow::response(400);

    if (db.validateUser(std::string(data["email"]), std::string(data["password"]))) // TODO: Hash password first
    {
        // Successful log in
        std::cout << "Logged in\n";
    }

    return crow::response(200);
}

crow::response WebServer::signup(const crow::request& req)
{
    auto data = crow::json::load(req.body);
    if (!data || !(data.has("username") && data.has("email") && data.has("password")))
        return crow::response(400);
    
    struct User user
    {
        std::string(data["username"]),
        std::string(data["email"]),
        std::string(data["password"]),
    };

    if (db.registerUser(user)) // TODO: Hash password first
    {
        // Successful registration
        std::cout << "Registered\n";
    }

    return crow::response(200);
}

void WebServer::setupRoutes()
{
    CROW_ROUTE(this->app, "/")
    ([this] 
    {
        std::cout << "Root Page\n";
        return rootPage();
    });

    CROW_ROUTE(this->app, "/login")
    ([this] 
    {
        return loginPage();
    });

    CROW_ROUTE(this->app, "/api/login")
        .methods("POST"_method)
        (
            [this](const crow::request& req)
            {
                return login(req);
            }
        );

    CROW_ROUTE(this->app, "/api/signup")
        .methods("POST"_method)
        (
            [this](const crow::request& req)
            {
                return signup(req);
            }
        );
}

WebServer::~WebServer()
{
    stop();
}