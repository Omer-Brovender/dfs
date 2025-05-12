#include "Database.hpp"
#include <sqlite3.h>

#ifdef _WIN32
    #include <sha.h>
#else
    #include <openssl/sha.h>
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <unordered_map>

constexpr int randMin = 1e6;
constexpr int randMax = 1e7 - 1;

std::string sha256(const std::string& s)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, s.c_str(), s.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

Database::Database(const char* filename)
{
    int exitc = sqlite3_open(filename, &this->db);

    std::string sql = "CREATE TABLE IF NOT EXISTS users("
                        "id         INTEGER   PRIMARY KEY   AUTOINCREMENT,           "
                        "username   TEXT                    NOT NULL        UNIQUE,  "
                        "passhash   TEXT                    NOT NULL,                "
                        "email      TEXT                    NOT NULL        UNIQUE,  "
                        "session    TEXT                                    UNIQUE   "
                        ");";

    std::string failMessage = "Couldn't create table!";
    std::string sccMessage = "Table successfully initiallized.";

    execQuery(
        sql,
        failMessage,
        sccMessage
    );

    sql = "CREATE TABLE IF NOT EXISTS files("
                    "id         INTEGER   PRIMARY KEY   AUTOINCREMENT,   "
                    "ownerid    INT                     NOT NULL,        "
                    "name       TEXT                    NOT NULL         "
                    ");";
    
    execQuery(
        sql,
        failMessage,
        sccMessage
    );
};

bool Database::rowExists(std::string& sql)
{
    struct sqlite3_stmt *selectstmt;
    int result = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &selectstmt, NULL);
    
    bool found = false;
    if (result == SQLITE_OK)
    {
        found = (sqlite3_step(selectstmt) == SQLITE_ROW);
    }
    sqlite3_finalize(selectstmt);

    return found;
}

bool Database::execQuery(std::string& sql, std::string& failMessage, std::string& sccMessage)
{
    char* errMessage;
    int exitc = sqlite3_exec(this->db, sql.c_str(), NULL, 0, &errMessage);

    if (exitc != SQLITE_OK) 
    { 
        std::cerr << failMessage << " (Err: " << errMessage << ")\n"; 
        sqlite3_free(errMessage); 
        return false;
    } 
    std::cout << sccMessage << "\n"; 
    return true;
}

User Database::getUser(std::string sessionID)
{
    std::string sql = "SELECT * FROM users"
                    " WHERE session='" + sessionID
                    + "';";

    struct sqlite3_stmt *selectstmt;
    int result = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &selectstmt, NULL);
    User ret;

    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to find user. (Err: " << sqlite3_errmsg(db) << ")\n";
        ret = User();
    }
    
    if (sqlite3_step(selectstmt) == SQLITE_ROW) 
    {
        int id = sqlite3_column_int(selectstmt, 0);
        auto username = std::string(reinterpret_cast<const char*> (sqlite3_column_text(selectstmt, 1)));
        auto passhash = std::string(reinterpret_cast<const char*> (sqlite3_column_text(selectstmt, 2)));
        auto email    = std::string(reinterpret_cast<const char*> (sqlite3_column_text(selectstmt, 3)));

        ret = User{username, email, passhash, id};
    } 
    else 
    {
        std::cout << "No row found for sessionID: " << sessionID << "\n";
    }

    sqlite3_finalize(selectstmt);
    return ret;
}

bool Database::registerUser(struct User user)
{
    std::string sql = "INSERT INTO users (username, passhash, email)"
                        " VALUES ('" 
                        + user.username + "', '" 
                        + user.passHash + "', '" 
                        + user.email 
                        + "');";

    std::string failMessage = "Failed to register user.";
    std::string sccMessage = "User successfuly registered.";

    return execQuery(
        sql,
        failMessage,
        sccMessage
    );
}

bool Database::validateUser(std::string email, std::string passHash)
{
    std::string sql = "SELECT * FROM users"
                        " WHERE email='" + email
                        + "' AND passhash='" + passHash 
                        + "';";

    return rowExists(sql);
}

int Database::registerFileUpload(int ownerID, std::string filename)
{
    std::string sql = "INSERT INTO files (ownerid, name)"
                        " VALUES ('" 
                        + std::to_string(ownerID) + "', '" 
                        + filename
                        + "');";

    std::string failMessage = "Failed to register user.";
    std::string sccMessage = "User successfuly registered.";

    execQuery(
        sql,
        failMessage,
        sccMessage
    );

    int id = sqlite3_last_insert_rowid(this->db);
    return id;
}

std::unordered_map<int, std::string> Database::getFiles(int ownerID)
{
    std::string sql = "SELECT * FROM files"
                    " WHERE ownerid='" + std::to_string(ownerID)
                    + "';";

    struct sqlite3_stmt *selectstmt;
    int result = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &selectstmt, NULL);
    std::unordered_map<int, std::string> files;

    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to obtain user files. (Err: " << sqlite3_errmsg(db) << ")\n";
    }
    
    while (sqlite3_step(selectstmt) == SQLITE_ROW) 
    {
        int id = sqlite3_column_int(selectstmt, 0);
        ownerID = sqlite3_column_int(selectstmt, 1);
        auto filename = std::string(reinterpret_cast<const char*> (sqlite3_column_text(selectstmt, 2)));

        files[id] = filename;
    }

    sqlite3_finalize(selectstmt);
    return files;
}

std::string Database::generateSession(std::string email)
{
    std::srand(std::time({}));
    
    int sessionIDNum = std::rand() % (randMax - randMin + 1) + randMin;
    std::string sessionID = sha256(std::to_string(sessionIDNum));
    
    std::string sql = "SELECT * FROM users"
                        " WHERE session='" + sessionID
                        + "';";
    
    bool exists = rowExists(sql);

    if (exists)
        return generateSession(email);

    sql = "UPDATE users"
            " SET session = '" + sessionID + "'"
            " WHERE email='" + email + "'";
    
    std::string failMessage = "Failed to update session.";
    std::string sccMessage = "Successfuly updated session.";

    execQuery(
        sql,
        failMessage,
        sccMessage
    );

    return sessionID;
}

bool Database::validateSession(std::string sessionID)
{
    std::string sql = "SELECT * FROM users"
                        " WHERE session='" + sessionID
                        + "';";
    
    return rowExists(sql);
}

void Database::close()
{
    sqlite3_close(this->db);
}

Database::~Database()
{
    close();
}