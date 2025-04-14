#include "FileUtils.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

std::vector<char> FileUtils::readFile(std::string path)
{
    std::cout << "F: " << path << "\n";
    std::ifstream file( std::filesystem::path(path.c_str()), std::ifstream::in | std::ifstream::binary );
    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const auto size = std::filesystem::file_size(path);
    data.resize(size);
    file.close();

    return data;
}