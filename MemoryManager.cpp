#include "headers/MemoryManager.h"

#include <algorithm>
#include <iomanip>
#include <mutex>

MemoryManager::MemoryManager(int maxMemory, int frameSize, int memoryPerProcess)
    : maxMemory(maxMemory), frameSize(frameSize), memoryPerProcess(memoryPerProcess) {
    memoryBlocks.push_back({0, maxMemory - 1, ""}); // Initial free block
}

bool MemoryManager::allocateMemory(const std::string &processName, int processSize) {
    int index = findFirstFit(processSize);
    if (index == -1) {
        return false;
    }

    MemoryBlock &block = memoryBlocks[index];
    if (block.end - block.start + 1 == processSize) {
        block.processName = processName;
    } else {
        MemoryBlock newBlock = {block.start, block.start + processSize - 1, processName};
        block.start += processSize;
        memoryBlocks.insert(memoryBlocks.begin() + index, newBlock);
    }
    return true;
}

void MemoryManager::deallocateMemory(const std::string &processName) {
    for (auto it = memoryBlocks.begin(); it != memoryBlocks.end(); ++it) {
        if (it->processName == processName) {
            it->processName = "";
            if (it != memoryBlocks.begin() && (it - 1)->processName.empty()) {
                it->start = (it - 1)->start;
                memoryBlocks.erase(it - 1);
                --it;
            }
            if (it != memoryBlocks.end() - 1 && (it + 1)->processName.empty()) {
                it->end = (it + 1)->end;
                memoryBlocks.erase(it + 1);
            }
            break;
        }
    }
}

bool MemoryManager::isProcessInMemory(const std::string &processName) {
    for (const auto &block : memoryBlocks) {
        if (block.processName == processName) {
            return true;
        }
    }
    return false;
}

void MemoryManager::generateReport(const std::string &filename) {
    std::ofstream reportFile(filename, std::ios::app);
    if (!reportFile.is_open()) {
        throw std::runtime_error("Could not open report file");
    }

    std::time_t now = std::time(nullptr);
    reportFile << "Timestamp: " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n";
    reportFile << "Number of processes in memory: " << std::count_if(memoryBlocks.begin(), memoryBlocks.end(), [](const MemoryBlock &block) { return !block.processName.empty(); }) << "\n";
    reportFile << "Total external fragmentation: " << calculateExternalFragmentation() / 1024 << " KB\n";
    reportFile << "Memory Layout: ";
    for (const auto &block : memoryBlocks) {
        if (block.processName.empty()) {
            reportFile << "[Free: " << block.start << "-" << block.end << "]";
        } else {
            reportFile << "[Process " << block.processName << ": " << block.start << "-" << block.end << "]";
        }
    }
    reportFile << "\n\n";
    reportFile.close();
}

int MemoryManager::findFirstFit(int processSize) {
    for (size_t i = 0; i < memoryBlocks.size(); ++i) {
        if (memoryBlocks[i].processName.empty() && memoryBlocks[i].end - memoryBlocks[i].start + 1 >= processSize) {
            return i;
        }
    }
    return -1;
}

void MemoryManager::VisualizeMemory() {
    for (const auto &block : memoryBlocks) {
        if (block.processName.empty()) {
            std::cout << "[Free: " << block.start << "-" << block.end << "]";
        } else {
            std::cout << "[Process " << block.processName << ": " << block.start << "-" << block.end << "]";
        }
    }
    std::cout << std::endl;
}

int MemoryManager::calculateExternalFragmentation() {
    int fragmentation = 0;
    for (const auto &block : memoryBlocks) {
        if (block.processName.empty()) {
            fragmentation += block.end - block.start + 1;
        }
    }
    return fragmentation;
}