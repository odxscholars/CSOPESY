#include "headers/MemoryManager.h"

#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <vector>

MemoryManager::MemoryManager(int maxMemory, int frameSize,
                             int minMemoryPerProcess, int maxMemoryPerProcess,
                             int memPerFrame)
    : maxMemory(maxMemory), frameSize(frameSize),
      minMemoryPerProcess(minMemoryPerProcess), memPerFrame(memPerFrame) {
  memoryBlocks.push_back({0, maxMemory - 1, ""}); // Initial free block

  for (int i = 1; i <= memPerFrame; i++) {
    Frame newFrame = {"", -1};
    processFrameMap.insert({i, newFrame});
    freeFrameList.push_back(i);
  }
}

int MemoryManager::pagingAllocate(const std::string &processName,
                                  int processSize, int processPageReq) {
  // take the first one from freeFrameList
  if (freeFrameList.empty()) {
    return -1;
  }
  // TODO mutate process' pages

  for (int i = 1; i < processPageReq; i++) {
    int page = freeFrameList.front();
    freeFrameList.erase(freeFrameList.begin());

    processFrameMap[page].processName = processName;
    processFrameMap[page].processPage = page;
  }

  return 1;
}

std::vector<int>
MemoryManager::findProcessInMap(const std::string &processName) {
  std::vector<int> pages;
  for (const auto &pair : processFrameMap) {
    if (pair.second.processName == processName) {
      pages.push_back(pair.first);
    }
  }
  return pages;
}

int MemoryManager::pagingDeallocate(const std::string &processName,
                                    int processPageAmt) {

  std::vector<int> pages = findProcessInMap(processName);

  if (pages.empty()) {
    return -1;
  }

  for (const int page : pages) {

    processFrameMap[page].processName = "";
    processFrameMap[page].processPage = -1;

    freeFrameList.push_back(page);
  }
  // TODO remove process pages

  return 1;
}

bool MemoryManager::allocateMemory(const std::string &processName,
                                   int processSize) {
  int index = findFirstFit(processSize);
  if (index == -1) {
    return false;
  }

  MemoryBlock &block = memoryBlocks[index];
  if (block.end - block.start + 1 == processSize) {
    block.processName = processName;
  } else {
    MemoryBlock newBlock = {block.start, block.start + processSize - 1,
                            processName};
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
  std::string folderPath =
      "./memory_stamps"; // Relative path, or use "/memory_stamps" for absolute
                         // path
  if (!std::filesystem::exists(folderPath)) {
    std::filesystem::create_directory(folderPath);
  }

  std::string filePath = folderPath + "/" + filename;
  std::ofstream reportFile(filePath);
  if (!reportFile.is_open()) {
    throw std::runtime_error("Could not open report file");
  }

  time_t now = std::time(nullptr);
  reportFile << "Timestamp: "
             << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
             << std::endl;
  reportFile << "Number of processes in memory: "
             << std::count_if(memoryBlocks.begin(), memoryBlocks.end(),
                              [](const MemoryBlock &block) {
                                return !block.processName.empty();
                              })
             << std::endl;
  reportFile << "Total external fragmentation: "
             << calculateExternalFragmentation() << " KB" << std::endl;
  reportFile << "Memory Layout: " << std::endl;
  reportFile << "----start---- = 0" << std::endl;
  for (const auto &block : memoryBlocks) {
    if (block.processName.empty()) {
      // reportFile << "[Free: " << block.start << "-" << block.end << "]";
      reportFile << "\n";
    } else {
      reportFile << block.start << std::endl;
      reportFile << block.processName << std::endl;
      reportFile << block.end << "\n" << std::endl;
    }
  }
  reportFile << "----end---- = " << maxMemory << std::endl;
  reportFile << "\n\n";
  reportFile.close();
}

int MemoryManager::findFirstFit(int processSize) {
  for (size_t i = 0; i < memoryBlocks.size(); ++i) {
    if (memoryBlocks[i].processName.empty() &&
        memoryBlocks[i].end - memoryBlocks[i].start + 1 >= processSize) {
      return i;
    }
  }
  return -1;
}

void MemoryManager::VisualizeMemory() {
  time_t now = std::time(nullptr);
  std::cout << "Timestamp: "
            << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
            << std::endl;
  std::cout << "Number of processes in memory: "
            << std::count_if(memoryBlocks.begin(), memoryBlocks.end(),
                             [](const MemoryBlock &block) {
                               return !block.processName.empty();
                             })
            << std::endl;
  std::cout << "Total external fragmentation: "
            << calculateExternalFragmentation() << " KB" << std::endl;
  std::cout << "Memory Layout: " << std::endl;
  std::cout << "----start---- = 0" << std::endl;
  for (const auto &block : memoryBlocks) {
    if (block.processName.empty()) {
      // std::cout << "[Free: " << block.start << "-" << block.end << "]";
      std::cout << "\n";
    } else {
      std::cout << block.start << std::endl;
      std::cout << block.processName << std::endl;
      std::cout << block.end << "\n" << std::endl;
    }
  }
  std::cout << "----end---- = " << maxMemory << std::endl;
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

std::string MemoryManager::getProcessMemoryBlocks() {
  std::ostringstream report;
  for (const auto &block : memoryBlocks) {
    if (!block.processName.empty()) {
      report << block.processName << " " << block.end - block.start << "MiB\n";
    }
  }
  return report.str();
}

int MemoryManager::getMemoryUsage() {
  int totalMemory;

  /*for (const auto &block : memoryBlocks) {*/
  /*  if (!block.processName.empty()) {*/
  /*    totalMemory += block.end - block.start;*/
  /*  }*/
  /*}*/
  return totalMemory;
}

double MemoryManager::getMemoryUtil() {
  double totalMemoryUtil;
  int totalMemory;

  /*for (const auto &block : memoryBlocks) {*/
  /*  if (!block.processName.empty()) {*/
  /*    totalMemory += block.end - block.start;*/
  /*  }*/
  /*}*/

  totalMemoryUtil = (static_cast<double>(totalMemory) / maxMemory) * 100;
  return totalMemoryUtil;
}
