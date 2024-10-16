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
    const auto size = std::filesystem::file_size(path);
    data.resize(size);
    return data;
}

std::vector<uint64_t> MasterNode::splitData(uint64_t dataSize, uint64_t chunkSize)
{
    std::vector<uint64_t> ranges;
    for (int c = 0; c < dataSize; c += chunkSize) ranges.push_back(c);
    ranges.push_back(dataSize);
    return ranges;
}