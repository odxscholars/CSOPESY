#include "headers/Config.h"
#include <fstream>
#include <iostream>

void Config::loadConfig(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw ConfigException("Could not open config file: " + filename);
    }

    std::map<std::string, bool> requiredParams = {
        {"num-cpu", false},
        {"scheduler", false},
        {"quantum-cycles", false},
        {"batch-process-freq", false},
        {"min-ins", false},
        {"max-ins", false},
        {"delays-per-exec", false},
        {"max-overall-mem", false},
        {"mem-per-frame", false},
        {"mem-per-proc", false}};

    std::string param;
    while (file >> param)
    {
        if (param == "num-cpu")
        {
            file >> numCPU;
            requiredParams[param] = true;
        }
        else if (param == "scheduler")
        {
            file >> schedulerType;
            requiredParams[param] = true;
        }
        else if (param == "quantum-cycles")
        {
            file >> quantumCycles;
            requiredParams[param] = true;
        }
        else if (param == "batch-process-freq")
        {
            file >> batchProcessFreq;
            requiredParams[param] = true;
        }
        else if (param == "min-ins")
        {
            file >> minInstructions;
            requiredParams[param] = true;
        }
        else if (param == "max-ins")
        {
            file >> maxInstructions;
            requiredParams[param] = true;
        }
        else if (param == "delays-per-exec")
        {
            file >> delaysPerExec;
            requiredParams[param] = true;
        }
        else if (param == "max-overall-mem")
        {
            file >> maxOverallMem;
            requiredParams[param] = true;
        }
        else if (param == "mem-per-frame")
        {
            file >> memPerFrame;
            requiredParams[param] = true;
        }
        else if (param == "mem-per-proc")
        {
            file >> memPerProc;
            requiredParams[param] = true;
        }
        else
        {
            throw ConfigException("Unknown parameter: " + param);
        }
    }

    // Check if all required parameters were found
    for (const auto &param : requiredParams)
    {
        if (!param.second)
        {
            throw ConfigException("Missing required parameter: " + param.first);
        }
    }

    initialized = true;
}

void Config::displayConfig() const {
    std::cout << "Number of CPUs: " << numCPU << std::endl;
    std::cout << "Scheduler: " << schedulerType << std::endl;
    std::cout << "Quantum Cycles: " << quantumCycles << std::endl;
    std::cout << "Batch Process Frequency: " << batchProcessFreq << std::endl;
    std::cout << "Minimum Instructions: " << minInstructions << std::endl;
    std::cout << "Maximum Instructions: " << maxInstructions << std::endl;
    std::cout << "Delays Per Execution: " << delaysPerExec << std::endl;
    std::cout << "Max Overall Memory: " << maxOverallMem << std::endl;
    std::cout << "Memory Per Frame: " << memPerFrame << std::endl;
    std::cout << "Memory Per Process: " << memPerProc << std::endl;
}