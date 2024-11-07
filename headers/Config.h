#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

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

private:
    std::string filename;
    int numCpu;
    std::string scheduler;
    int quantumCycles;
    int batchProcessFreq;
    int minIns;
    int maxIns;
    int delaysPerExec;

    void parseLine(const std::string &line);

};