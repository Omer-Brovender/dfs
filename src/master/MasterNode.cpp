#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include "MasterNode.hpp"

MasterNode::MasterNode()
{
    
}

std::vector<char> MasterNode::readFile(std::wstring path)
{
    std::ifstream file( std::filesystem::path(path.c_str()) );
    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return data;
}