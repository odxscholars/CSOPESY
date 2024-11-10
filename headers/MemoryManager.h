#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

class MemoryManager {
public:
    MemoryManager(int maxMemory, int frameSize, int memoryPerProcess);
    bool allocateMemory(const std::string &processName, int processSize);
    void deallocateMemory(const std::string &processName);
    bool isProcessInMemory(const std::string &processName);
    void generateReport(const std::string &filename);

    void VisualizeMemory();
    int maxMemory;
    int frameSize;
    int memoryPerProcess;

private:
    struct MemoryBlock {
        int start;
        int end;
        std::string processName;
    };


    std::vector<MemoryBlock> memoryBlocks;
    int findFirstFit(int processSize);


    int calculateExternalFragmentation();
};

#endif // MEMORYMANAGER_H