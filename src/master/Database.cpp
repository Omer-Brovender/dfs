#include "Database.hpp"
#include <sqlite3.h>
#include <iostream>

static bool userFound = false;

static int validateUserCallback(void* data, int argc, char** argv, char** azColName) 
{ 
    userFound = true;
    return 0; 
} 

Database::Database(const char* filename)
{
    int exitc = sqlite3_open(filename, &this->db);

    std::string sql = "CREATE TABLE IF NOT EXISTS users("
                        "id         INT   PRIMARY KEY,          "
                        "username   TEXT                NOT NULL   UNIQUE, "
                        "passhash   TEXT                NOT NULL,          "
                        "email      TEXT                NOT NULL   UNIQUE  "
                        ");";

    char* errMessage;
    exitc = sqlite3_exec(this->db, sql.c_str(), NULL, 0, &errMessage);

    if (exitc != SQLITE_OK) 
    { 
        std::cerr << "Couldn't create table! Err: " << std::string(errMessage) << "\n"; 
        sqlite3_free(errMessage); 
    } 
    else
        std::cout << "Table successfully initiallized.\n"; 
};

bool Database::registerUser(struct User user)
{
    std::string sql = "INSERT INTO users (username, passhash, email)"
                        " VALUES ('" 
                        + user.username + "', '" 
                        + user.passHash + "', '" 
                        + user.email 
                        + "');";

    char* errMessage;
    int exitc = sqlite3_exec(this->db, sql.c_str(), NULL, 0, &errMessage);

    if (exitc != SQLITE_OK) 
    { 
        std::cerr << "Couldn't add user! Err: " << errMessage << "\n"; 
        sqlite3_free(errMessage); 
        return false;
    } 
    std::cout << "User successfully registered.\n"; 
    return true;
}

bool Database::validateUser(std::string email, std::string passHash)
{
    std::string sql = "SELECT * FROM users"
                        " WHERE email='" + email
                        + "' AND passhash='" + passHash 
                        + "';";
    
    char* errMessage;
    int exitc = sqlite3_exec(this->db, sql.c_str(), &validateUserCallback, 0, &errMessage);

    if (exitc != SQLITE_OK) 
    { 
        std::cerr << "Couldn't validate login! Err: " << errMessage << "\n"; 
        sqlite3_free(errMessage); 
        return false;
    } 
    std::cout << "Successfully verified login.\n"; 

    bool ret = userFound;
    userFound = false;
    return ret;
}

void Database::close()
{
    sqlite3_close(this->db);
}

Database::~Database()
{
    close();
}