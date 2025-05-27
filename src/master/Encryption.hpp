#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

#include <vector>

namespace Encryption
{
	bool encrypt(const std::vector<unsigned char>& plainText, std::vector<unsigned char>& cipherText, const unsigned char* key);
	bool encrypt(const std::string& plainText, std::string& cipherText, const unsigned char* key);

	bool decrypt(const std::vector<unsigned char>& cipherText, std::vector<unsigned char>& plainText, const unsigned char* key);
	bool decrypt(const std::string& cipherText, std::string& plainText, const unsigned char* key);

	std::string sha256(const std::string& s);
}

#endif
