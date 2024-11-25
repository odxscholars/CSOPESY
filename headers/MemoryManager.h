#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class MemoryManager {
public:
  MemoryManager(int maxMemory, int frameSize, int memoryPerProcess);
  bool allocateMemory(const std::string &processName, int processSize);
  void deallocateMemory(const std::string &processName);
  bool isProcessInMemory(const std::string &processName);
  void generateReport(const std::string &filename);
  std::string getProcessMemoryBlocks();
  double getMemoryUtil();
  int getMemoryUsage();

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
