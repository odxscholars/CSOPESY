#include "headers/MemoryManager.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <pstl/glue_algorithm_defs.h>
#include <string>
#include <unordered_map>
#include <vector>

MemoryManager::MemoryManager(int maxMemory, int frameSize,
                             int minMemoryPerProcess, int maxMemoryPerProcess,
                             int memPerFrame)
    : maxMemory(maxMemory), frameSize(frameSize),
      minMemoryPerProcess(minMemoryPerProcess), memPerFrame(memPerFrame) {
  memoryBlocks.push_back({0, maxMemory - 1, ""}); // Initial free block
  int numFrames = maxMemory / frameSize;
  for (int i = 1; i <= numFrames; i++) {

    Frame newFrame = {nullptr, "", -1, 0};
    std::cout << "Frame: " << i << std::endl;
    processFrameMap.insert({i, newFrame});
    freeFrameList.push_back(i);
  }
}

/*
 * process - process ptr
 * processSize - total ProcessSize
 * processPageReq - total pages required for the process. Formula: processSize /
 * frameSize
 */
bool MemoryManager::pagingAllocate(Process *process, int processPageReq) {
  // get unix timestamp
  std::lock_guard<std::mutex> lock(frameMutex);
  std::time_t timestamp = std::time(nullptr);
  int processSize = process->getProcessSize();

  // std::cout << "Allocating " << processPageReq << " pages for process "
  //           << process->getProcessName() << std::endl;

  if (freeFrameList.empty()) {
    return false;
  }

  if (processPageReq == 1) {
    int page = freeFrameList.front();
    freeFrameList.erase(freeFrameList.begin());

    processFrameMap[page].processSizeInMem = processSize;
    processFrameMap[page].processName = process->getProcessName();
    processFrameMap[page].processPage = page;
    processFrameMap[page].timestamp = timestamp;
    processFrameMap[page].processPtr = process;

    // std::cout << "Allocated page " << page << " for process "
    //           << process->getProcessName() << std::endl;

    process->pages.push_back(page);
    pagedIns += 1;
    return true;
  }

  for (int i = 1; i <= processPageReq; i++) {
    int currSize = processSize - memPerFrame;
    int page = freeFrameList.front();
    freeFrameList.erase(freeFrameList.begin());

    processFrameMap[page].processName = process->getProcessName();
    processFrameMap[page].processPage = page;
    processFrameMap[page].timestamp = timestamp;
    processFrameMap[page].processPtr = process;
    if (currSize < 0) {
      processFrameMap[page].processSizeInMem = processSize;
    } else {
      processFrameMap[page].processSizeInMem = currSize;
    }

    /*std::cout << "Mem size for: " << processFrameMap[page].processSizeInMem*/
    /*          << " for process " << process->getProcessName() << std::endl;*/

    process->pages.push_back(page);
    pagedIns += 1;
  }

  return true;
}

Process *MemoryManager::getOldestProcessInFrameMap() {
  std::lock_guard<std::mutex> lock(frameMutex);
  std::time_t oldestTimestamp = std::time(nullptr);
  Process *oldestProcess = nullptr;
  // traverse through the processFrameMap to find the oldest process
  for (const auto &pair : processFrameMap) {
    if (pair.second.timestamp < oldestTimestamp && !pair.second.processPtr->isRunning) {
      oldestTimestamp = pair.second.timestamp;
      oldestProcess = pair.second.processPtr;
    }
  }
  return oldestProcess;
}

std::vector<int>
MemoryManager::findProcessInMap(const std::string &processName) {
  std::lock_guard<std::mutex> lock(frameMutex);
  std::vector<int> pages;
  for (const auto &pair : processFrameMap) {
    if (pair.second.processName == processName) {
      pages.push_back(pair.first);
    }
  }
  return pages;
}

bool MemoryManager::pagingDeallocate(Process *process) {
  std::vector<int> pages = findProcessInMap(process->getProcessName());

  if (pages.empty()) {
    return false;
  }

  for (const int page : pages) {
    processFrameMap[page].processName = "";
    processFrameMap[page].processPage = -1;
    processFrameMap[page].timestamp = 0;
    processFrameMap[page].processPtr = nullptr;

    freeFrameList.push_back(page);
    auto newEnd =
        std::remove(process->pages.begin(), process->pages.end(), page);

    process->pages.erase(newEnd, process->pages.end());
    pagedOuts += 1;
  }

  return true;
}

void MemoryManager::writeBackingStore(Process *process) {
  std::cout << "IM GETTING TRIGGERED!";
  std::string fileName = "backing_store.txt";
  std::ofstream file;

  file.open(fileName, std::ios::out);

  if (!file) {
    std::cerr << "Could not open file!";
  }

  file << "\n";
  file << process->getProcessName() << "\t";
  file << process->getInstructionsDone();

  file.close();
}

bool MemoryManager::isProcessinPagingMemory(Process *process) {
  if (process->pages.size() == 0) {
    return false;
  }
  return true;
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
  for (const auto &frame : processFrameMap) {
    if (!(frame.second.processPtr == nullptr)) {
      report << frame.second.processName << " " << frame.second.processSizeInMem
             << "Mib\n";
    }
  }
  return report.str();
}

int MemoryManager::getMemoryUsage(const std::string &memoryType) {
  std::lock_guard<std::mutex> lock(frameMutex);
  int totalMemory = 0;

  if (memoryType == "flat") {
    for (const auto &block : memoryBlocks) {
      if (!block.processName.empty()) {
        totalMemory += block.end - block.start + 1;
      }
    }
  } else {
    for (const auto &frame : processFrameMap) {
      if (frame.second.processPtr != nullptr) {
        totalMemory += memPerFrame;
      }
    }
  }

  return totalMemory;
}

double MemoryManager::getMemoryUtil(const std::string &memoryType) {
  std::lock_guard<std::mutex> lock(frameMutex);
  std::unordered_map<int, Frame> tempProcessFrameMap = processFrameMap;
  double totalMemoryUtil;
  int totalMemory = 0;

  if (memoryType == "flat") {
    for (const auto &block : memoryBlocks) {
      if (!block.processName.empty()) {
        totalMemory += frameSize;
      }
    }
  } else {
    for (const auto &frame : tempProcessFrameMap) {
      if (!(frame.second.processPtr == nullptr)) {
        totalMemory += frameSize;
      }
    }
  }

  totalMemoryUtil = (static_cast<double>(totalMemory) / maxMemory) * 100;
  return totalMemoryUtil;
}

int MemoryManager::getFreeMemory(const std::string &memoryType) {
  std::lock_guard<std::mutex> lock(frameMutex);
  int freeMemory = 0;
  std::unordered_map<int, Frame> tempProcessFrameMap = processFrameMap;

  if (memoryType == "flat") {
    for (const auto &block : memoryBlocks) {
      if (block.processName.empty()) {
        freeMemory += frameSize;
      }
    }
  } else {
    for (const auto &frame : tempProcessFrameMap) {
      if (frame.second.processPtr == nullptr) {
        freeMemory += frameSize;
      }
    }
  }

  return freeMemory;
}

void MemoryManager::visualizeFrames() {
  std::cout << "+---------+----------------+----------------+------------------"
               "---+--------------+"
            << std::endl;
  std::cout << "| Frame # | Process Name   | Process Page   | Timestamp        "
               "   | Process Size |"
            << std::endl;
  std::cout << "+---------+----------------+----------------+------------------"
               "---+--------------+"
            << std::endl;

  for (const auto &pair : processFrameMap) {
    const Frame &frame = pair.second;
    std::cout << "| " << std::setw(7) << pair.first << " | " << std::setw(14)
              << (frame.processName.empty() ? "Free" : frame.processName)
              << " | " << std::setw(14)
              << (frame.processPage == -1 ? "N/A"
                                          : std::to_string(frame.processPage))
              << " | " << std::setw(19)
              << (frame.timestamp == 0 ? "N/A"
                                       : std::to_string(frame.timestamp))
              << " | " << std::setw(12)
              << (frame.processPtr == nullptr
                      ? "N/A"
                      : std::to_string(frame.processSizeInMem))
              << " |" << std::endl;
  }

  std::cout << "+---------+----------------+----------------+------------------"
               "---+--------------+"
            << std::endl;


}

int MemoryManager::getFramesOccupiedByProcess(const std::string &processName) {
  int framesOccupied = 0;
  for (const auto &frame : processFrameMap) {
    if (frame.second.processName == processName) {
      framesOccupied++;
    }
  }
  return framesOccupied;
}