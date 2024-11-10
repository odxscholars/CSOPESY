#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <stdexcept>
#include <cstdint>
#include <map>

class Config
{
public:
    // This allows only one instance of Config to exist
    static Config &getInstance()
    {
        static Config instance;
        return instance;
    }

    void loadConfig(const std::string &filename);
    bool isInitialized() const { return initialized; }
    void displayConfig() const;

    // Getters
    int getNumCPU() const { return numCPU; }
    std::string getSchedulerType() const { return schedulerType; }
    uint32_t getQuantumCycles() const { return quantumCycles; }
    uint32_t getBatchProcessFreq() const { return batchProcessFreq; }
    uint32_t getMinInstructions() const { return minInstructions; }
    uint32_t getMaxInstructions() const { return maxInstructions; }
    uint32_t getDelaysPerExec() const { return delaysPerExec; }
    //week 8
    uint16_t getMaxOverallMem() const { return maxOverallMem; }
    uint16_t getMemPerFrame() const { return memPerFrame; }
    uint16_t getMemPerProc() const { return memPerProc; }

    // Exception class for Config
    class ConfigException : public std::runtime_error
    {
    public:
        ConfigException(const std::string &msg) : std::runtime_error(msg) {}
    };

private:
    Config() : initialized(false) {}

    int numCPU;                
    std::string schedulerType; 
    uint32_t quantumCycles;    
    uint32_t batchProcessFreq; 
    uint32_t minInstructions;  
    uint32_t maxInstructions;  
    uint32_t delaysPerExec;    
    //week 8
    uint16_t maxOverallMem;    
    uint16_t memPerFrame;      
    uint16_t memPerProc;       

    bool initialized;

    void validateParameters();
};

#endif