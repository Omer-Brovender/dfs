#ifndef MASTERNODE_HPP
#define MASTERNODE_HPP

#include <string>
#include <vector>

class MasterNode
{
//private:
public:
    std::vector<char> readFile(std::wstring path);
    std::vector<uint64_t> splitData(uint64_t dataSize, uint64_t chunkSize);


    MasterNode();

};

#endif