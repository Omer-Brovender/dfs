#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <crow/utility.h>

#include "Encryption.hpp"
#define AES_BLOCK_SIZE 16 // an AES block is always 16 bytes (128 bits?) long

const int AES_KEY_LENGTH = 32; // 256 bits
const int AES_IV_LENGTH = 16;  // 128 bits

bool Encryption::encrypt(const std::vector<unsigned char>& plainText, std::vector<unsigned char>& cipherText, const unsigned char* key) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    cipherText.resize(plainText.size() + AES_BLOCK_SIZE);
    int len = 0, cipherText_len = 0;

    if (1 != EVP_EncryptUpdate(ctx, cipherText.data(), &len, plainText.data(), plainText.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    cipherText_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, cipherText.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    cipherText_len += len;
    cipherText.resize(cipherText_len);

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool Encryption::encrypt(const std::string& plainText, std::string& cipherText, const unsigned char* key)
{
    std::vector<unsigned char> plainTextBuffer(plainText.begin(), plainText.end());
    std::vector<unsigned char> cipherBuffer;

    if (!Encryption::encrypt(plainTextBuffer, cipherBuffer, key)) return false;

    cipherText = std::string(cipherBuffer.begin(), cipherBuffer.end());
    cipherText = crow::utility::base64encode(cipherText, cipherText.size());
    return true;
}

bool Encryption::decrypt(const std::vector<unsigned char>& cipherText, std::vector<unsigned char>& plainText, const unsigned char* key) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    plainText.resize(cipherText.size());
    int len = 0, plainText_len = 0;

    if (1 != EVP_DecryptUpdate(ctx, plainText.data(), &len, cipherText.data(), cipherText.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plainText_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plainText.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plainText_len += len;
    plainText.resize(plainText_len);

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool Encryption::decrypt(const std::string& cipherText, std::string& plainText, const unsigned char* key)
{
    std::vector<unsigned char> plainTextBuffer;
    std::string decoded = crow::utility::base64decode(cipherText, cipherText.size());
    std::vector<unsigned char> cipherBuffer(decoded.begin(), decoded.end());

    if (!Encryption::decrypt(cipherBuffer, plainTextBuffer, key)) return false;

    plainText = std::string(plainTextBuffer.begin(), plainTextBuffer.end());
    return true;
}


std::string Encryption::sha256(const std::string& s)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, s.c_str(), s.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}
