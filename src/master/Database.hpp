#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <winsqlite/winsqlite3.h> 
#include <unordered_map>

struct User 
{
    std::string username;
    std::string email;
    std::string passHash;
    int ID;
};

class Database
{
public:
    Database(const char* filename);
    bool registerUser(struct User user);
    bool validateUser(std::string email, std::string passHash);
    int registerFileUpload(int ownerID, std::string filename);
    std::unordered_map<int, std::string> getFiles(int ownerID);
    std::string generateSession(std::string email);
    bool validateSession(std::string sessionID);
    User getUser(std::string sessionID);
    void close();
    ~Database();

private:
    bool rowExists(std::string& sql);
    bool execQuery(std::string& sql, std::string& errMessage, std::string& sccMessage);
    void initialize();
    sqlite3* db;
};

#endif