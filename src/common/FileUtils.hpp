#ifndef FILEUTILS_HPP
#define FILEUTILS_HPP

#include <string>
#include <vector>

class FileUtils
{
public:
    static std::vector<char> readFile(std::string path);
};

#endif