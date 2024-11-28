#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

class Config {
public:
  Config(const std::string &filename);
  void loadConfig();
  void displayConfig() const;

  int getNumCpu() const;
  std::string getScheduler() const;
  int getQuantumCycles() const;
  int getBatchProcessFreq() const;
  int getMinIns() const;
  int getMaxIns() const;
  int getDelaysPerExec() const;
  int getMaxOverallMemory() const;
  int getMemoryPerFrame() const;
  int getMemoryPerProcess() const;

private:
  std::string filename;
  int numCpu;
  std::string scheduler;
  int quantumCycles;
  int batchProcessFreq;
  int minIns;
  int maxIns;
  int delaysPerExec;
  int maxOverallMemory;
  int memoryPerFrame;
  int memoryPerProcess;

  void parseLine(const std::string &line);
};
