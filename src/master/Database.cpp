#include "Database.hpp"
#include "Encryption.hpp"

#include <sqlite/sqlite3.h>

#include <iostream>
#include <sstream>
#include <ctime>
#include <unordered_map>

constexpr unsigned char AES_KEY[32] = {
    0xfa, 0x2e, 0xec, 0x1f, 0x45, 0xda, 0x79, 0xbe,
    0x2b, 0x73, 0xae, 0xf2, 0x65, 0x7e, 0x7c, 0x81,
    0x1f, 0x95, 0x1c, 0xc7, 0x1b, 0x0a, 0xb8, 0xd7,
    0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};

constexpr int randMin = 1e6;
constexpr int randMax = 1e7 - 1;

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
    std::string encryptedSession;
    Encryption::encrypt(sessionID, encryptedSession, AES_KEY);

    std::string sql = "SELECT * FROM users"
                    " WHERE session='" + encryptedSession
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

        std::string decryptedUsername;
        std::string decryptedPassHash;
        std::string decryptedEmail;

        Encryption::decrypt(username, decryptedUsername, AES_KEY);
        Encryption::decrypt(passhash, decryptedPassHash, AES_KEY);
        Encryption::decrypt(email,    decryptedEmail,    AES_KEY);

        ret = User{decryptedUsername, decryptedEmail, decryptedPassHash, id};
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
    std::string encryptedUsername;
    std::string encryptedPasshash;
    std::string encryptedEmail;

    Encryption::encrypt(user.username, encryptedUsername, AES_KEY);
    Encryption::encrypt(user.passHash, encryptedPasshash, AES_KEY);
    Encryption::encrypt(user.email,    encryptedEmail,    AES_KEY);

    std::string sql = "INSERT INTO users (username, passhash, email)"
                        " VALUES ('" 
                        + encryptedUsername + "', '" 
                        + encryptedPasshash + "', '" 
                        + encryptedEmail 
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
    std::string encryptedPasshash;
    std::string encryptedEmail;

    Encryption::encrypt(passHash, encryptedPasshash, AES_KEY);
    Encryption::encrypt(email, encryptedEmail, AES_KEY);

    std::string sql = "SELECT * FROM users"
                        " WHERE email='" + encryptedEmail
                        + "' AND passhash='" + encryptedPasshash 
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
    std::string sessionID = Encryption::sha256(std::to_string(sessionIDNum));

    std::string encryptedSession;
    Encryption::encrypt(sessionID, encryptedSession, AES_KEY);
    
    std::string sql = "SELECT * FROM users"
                        " WHERE session='" + encryptedSession
                        + "';";
    
    bool exists = rowExists(sql);

    if (exists)
        return generateSession(email);

    std::string encryptedEmail;
    Encryption::encrypt(email, encryptedEmail, AES_KEY);

    sql = "UPDATE users"
            " SET session = '" + encryptedSession + "'"
            " WHERE email='" + encryptedEmail + "'";
    
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
    std::string encryptedSession;
    Encryption::encrypt(sessionID, encryptedSession, AES_KEY);

    std::string sql = "SELECT * FROM users"
                        " WHERE session='" + encryptedSession
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