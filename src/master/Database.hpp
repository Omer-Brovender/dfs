#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <sqlite3.h> 

struct User 
{
    std::string username;
    std::string email;
    std::string passHash;
};

class Database
{
public:
    Database(const char* filename);
    bool registerUser(struct User user);
    bool validateUser(std::string email, std::string passHash);
    std::string generateSession(std::string email);
    bool validateSession(std::string sessionID);
    void close();
    ~Database();

private:
    bool rowExists(std::string& sql);
    bool execQuery(std::string& sql, std::string& errMessage, std::string& sccMessage);
    void initialize();
    sqlite3* db;
};

#endif