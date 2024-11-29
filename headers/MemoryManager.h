#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "Process.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class MemoryManager {
public:
  MemoryManager(int maxMemory, int frameSize, int minMemoryPerProcess,
                int maxMemoryPerProcess, int memPerFrame);

  bool pagingAllocate(Process *process, int processPageReq);

  Process *getOldestProcessInFrameMap();

  bool pagingDeallocate(Process *process);

  bool isProcessinPagingMemory(Process *process);

  bool allocateMemory(const std::string &processName, int processSize);
  void deallocateMemory(const std::string &processName);
  bool isProcessInMemory(const std::string &processName);
  void generateReport(const std::string &filename);
  std::string getProcessMemoryBlocks();
  int getMemoryUsage(const std::string &memoryType);
  double getMemoryUtil(const std::string &memoryType);
  int getFreeMemory(const std::string &memoryType);

  void visualizeFrames();

  void VisualizeMemory();
  int maxMemory;
  int frameSize;
  int minMemoryPerProcess;
  int maxMemoryPerProcess;
  int memPerFrame;
  int pagedIns = 0;
  int pagedOuts = 0;

  // Paging stuff

  struct Frame {
    Process *processPtr;
    std::string processName;
    int processPage;
    std::time_t timestamp = 0;
    int processSizeInMem = 0;
  };

  std::vector<int> freeFrameList;
  std::unordered_map<int, Frame> processFrameMap;

private:
  struct MemoryBlock {
    int start;
    int end;
    std::string processName;
  };

  std::vector<MemoryBlock> memoryBlocks;
  std::mutex frameMutex;

  int findFirstFit(int processSize);
  std::vector<int> findProcessInMap(const std::string &processName);
  int calculateExternalFragmentation();
};

#endif // MEMORYMANAGER_H
//
//
//
// 1. Processes will hold how big the required memory is
// 2. Allocator will compute for how many pages it requires based on
// mem-per-frame
// 3.
//
//
//
// PROCESS should have:
// -memory size (is random between max and min)
// -pages (list the pages it's in)
//
//
// FRAME should have:
// start
// end
// *process
// sizeConsumed
//
