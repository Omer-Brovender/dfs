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
    void close();
    ~Database();

private:
    void initialize();
    sqlite3* db;
};

#endif