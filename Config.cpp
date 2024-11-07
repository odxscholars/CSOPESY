#include "headers/Config.h"

Config::Config(const std::string &filename) : filename(filename) {}

void Config::loadConfig() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file or file does not exist");
    }

    std::string line;
    while (std::getline(file, line)) {
        parseLine(line);
    }
    file.close();
}

void Config::parseLine(const std::string &line) {
    std::istringstream iss(line);
    std::string key;
    if (iss >> key) {
        std::string value;
        if (iss >> value) {

            if (key == "num-cpu") {
                numCpu = std::stoi(value);
            } else if (key == "scheduler") {
                if (value.front() == '\"' && value.back() == '\"') {
                    scheduler = value.substr(1, value.size() - 2);
                } else {
                    scheduler = value;
                }
            } else if (key == "quantum-cycles") {
                quantumCycles = std::stoi(value);
            } else if (key == "batch-process-freq") {
                batchProcessFreq = std::stoi(value);
            } else if (key == "min-ins") {
                minIns = std::stoi(value);
            } else if (key == "max-ins") {
                maxIns = std::stoi(value);
            } else if (key == "delay-per-exec") {
                delaysPerExec = std::stoi(value);
            }
        }
    }
}
void Config::displayConfig() const {
    std::cout << "Number of CPUs: " << numCpu << std::endl;
    std::cout << "Scheduler: " << scheduler << std::endl;
    std::cout << "Quantum Cycles: " << quantumCycles << std::endl;
    std::cout << "Batch Process Frequency: " << batchProcessFreq << std::endl;
    std::cout << "Minimum Instructions: " << minIns << std::endl;
    std::cout << "Maximum Instructions: " << maxIns << std::endl;
    std::cout << "Delays Per Execution: " << delaysPerExec << std::endl;
}
int Config::getNumCpu() const {
    return numCpu;
}

std::string Config::getScheduler() const {
    return scheduler;
}

int Config::getQuantumCycles() const {
    return quantumCycles;
}

int Config::getBatchProcessFreq() const {
    return batchProcessFreq;
}

int Config::getMinIns() const {
    return minIns;
}

int Config::getMaxIns() const {
    return maxIns;
}

int Config::getDelaysPerExec() const {
    return delaysPerExec;
}