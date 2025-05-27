#include "WebServer.hpp"
#include "Database.hpp"
#include "crow/app.h"
#include "crow/ci_map.h"
#include "crow/http_request.h"
#include "crow/http_response.h"
#include "crow/json.h"
#include "crow/middlewares/cookie_parser.h"
#include "crow/mustache.h"
#include "crow/multipart.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include "Encryption.hpp"
#include "LoginAttemptTracker.hpp"

WebServer::WebServer(std::string webRoot, std::string dbPath)
: db(dbPath.c_str()), webRoot(webRoot)
{
    std::filesystem::current_path(this->webRoot / "..");
    crow::mustache::set_global_base(webRoot);
    setupRoutes();
}

void WebServer::setMasterNode(std::shared_ptr<MasterNode> master)
{
    this->master = master;
}

void WebServer::start(std::uint16_t port)
{
    this->app.port(port)
        .ssl_file( (webRoot / ".." / "ssl" / "Certificate.crt").string(), (webRoot / ".." / "ssl" / "Certificate.key").string())
        .multithreaded()
        .run();
}

void WebServer::stop()
{
    this->db.close();
    this->app.stop();
}

crow::response WebServer::rootPage(const crow::request& req)
{
    auto& ctx = this->app.get_context<crow::CookieParser>(req);
    auto session = ctx.get_cookie("sessionID");

    if (db.validateSession(session))
    {
        auto user = this->db.getUser(session);
        std::cout << "Username: " << user.username << "\n";
        return crow::mustache::load("templates/index.html").render();
    }
    crow::response res;
    res.redirect("/login");
    return res;
}

crow::response WebServer::loginPage(const crow::request& req)
{
    auto& ctx = this->app.get_context<crow::CookieParser>(req);
    auto session = ctx.get_cookie("sessionID");

    if (!db.validateSession(session))
        return crow::mustache::load("templates/login.html").render();

    crow::response res;
    res.redirect("/");
    return res;
}

crow::response WebServer::login(const crow::request& req)
{
    static LoginAttemptTracker attemptTracker;
    std::string ip = req.remote_ip_address;

    if (!attemptTracker.allowLogin(ip))
        return crow::response(429, "Too many failed attempts. Try again later.");

    auto data = crow::json::load(req.body);
    if (!data || !(data.has("email") && data.has("password")))
         return crow::response(400, "Missing email/password.");

    if (db.validateUser(std::string(data["email"]), Encryption::sha256(std::string(data["password"])))) // TODO: Salt password
    {
        attemptTracker.reset(ip);
        std::string session = db.generateSession(std::string(data["email"]));
        return crow::response(200, crow::json::wvalue({{"session", session}}));
    }

    attemptTracker.recordFailure(ip);
    return crow::response(401, "Email and/or password were incorrect");
}

crow::response WebServer::signup(const crow::request& req)
{
    auto data = crow::json::load(req.body);
    if (!data || !(data.has("username") && data.has("email") && data.has("password")))
        return crow::response(400, "Malformed Request");
    
    struct User user
    {
        std::string(data["username"]),
        std::string(data["email"]),
        Encryption::sha256(std::string(data["password"])),
    };

    if (db.registerUser(user)) // TODO: Hash password first
    {
        // Successful registration
        return crow::response(200, "Successfully Registered");
    }

    return crow::response(403, "Couldn't register, email/username weren't unique");
}

crow::response WebServer::upload(const crow::request& req)
{
    auto& ctx = this->app.get_context<crow::CookieParser>(req);
    auto session = ctx.get_cookie("sessionID");

    if (!db.validateSession(session))
        return crow::response(401);

    int ownerID = this->db.getUser(session).ID;

    crow::multipart::message message(req);

    auto data = message.part_map;
    if (data.size() != 1 || data.find("file") == data.end()) return crow::response(400); // Malformed request - no file/too many files
    std::string filename = "";
    try
    {
        filename = data.find("file")->second
                    .headers.find("Content-Disposition")->second
                    .params.find("filename")->second;
    }
    catch (int) 
    {
        return crow::response(400);
    }

    std::string fileData = data.find("file")->second.body;
    int id = db.registerFileUpload(ownerID, filename);
    std::vector<char> dataVector(fileData.cbegin(), fileData.cend());
    this->master->upload(dataVector, id);

    return crow::response(200);
}

crow::response WebServer::download(const crow::request& req, int id)
{
    auto& ctx = this->app.get_context<crow::CookieParser>(req);
    auto session = ctx.get_cookie("sessionID");

    if (!db.validateSession(session))
        return crow::response(401);

    int ownerID = this->db.getUser(session).ID;
    std::unordered_map<int, std::string> files = this->db.getFiles(ownerID);
    if (files.find(id) == files.end())
        return crow::response(401);

    std::vector<char> fileData = this->master->downloadFile(id);
    std::cout << "Size: " << fileData.size() << "\n";

    return crow::response(std::string(fileData.begin(), fileData.end()));
}

crow::response WebServer::files(const crow::request& req)
{
    auto& ctx = this->app.get_context<crow::CookieParser>(req);
    auto session = ctx.get_cookie("sessionID");

    if (!db.validateSession(session))
        return crow::response(401);

    int ownerID = this->db.getUser(session).ID;
    std::unordered_map<int, std::string> files = this->db.getFiles(ownerID);
    crow::json::wvalue ret;

    for (auto& [id, filename] : files)
    {
        std::cout << id << ": " << filename << "\n";
        ret[std::to_string(id)] = filename;
    }

    return ret;
}

void WebServer::setupRoutes()
{
    CROW_ROUTE(this->app, "/")
    ([this](const crow::request& req)
    {
        std::cout << "Root Page\n";
        return rootPage(req);
    });

    CROW_ROUTE(this->app, "/login")
    ([this](const crow::request& req)
    {
        return loginPage(req);
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
    
    CROW_ROUTE(this->app, "/api/upload")
        .methods("POST"_method)
        (
            [this](const crow::request& req)
            {
                return upload(req);
            }
        );
    
    CROW_ROUTE(this->app, "/api/files")
        .methods("GET"_method)
        (
            [this](const crow::request& req)
            {
                return files(req);
            }
        );

    CROW_ROUTE(this->app, "/api/download/<int>/<string>")
        .methods("GET"_method)
        (
            [this](const crow::request& req, int id, std::string s)
            {
                return download(req, id);
            }
        );
}

WebServer::~WebServer()
{
    stop();
}