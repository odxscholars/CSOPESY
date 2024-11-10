#include "ProcessManager.h"
#include "Utils.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>

void ProcessManager::generateProcess(const std::string &name)
{
    if (name.empty()) throw std::runtime_error("Process name cannot be empty");

    std::unique_lock<std::mutex> lock(processesMutex);
    if (processes.find(name) != processes.end()) throw std::runtime_error("Process with name '" + name + "' already exists");

    auto process = std::make_shared<Process>(nextPID++, name);
    processes[name] = process;
    lock.unlock();
    Scheduler::getInstance().addProcess(process);

}

std::shared_ptr<Process> ProcessManager::getProcess(const std::string &name)
{
    std::lock_guard<std::mutex> lock(processesMutex);
    auto it = processes.find(name);
    return (it != processes.end()) ? it->second : nullptr;
}

void ProcessManager::listProcesses()
{
    std::vector<std::shared_ptr<Process>> processSnapshot;
    int activeCount = 0, totalCores;

    {
        std::lock_guard<std::mutex> lock(processesMutex);
        totalCores = Config::getInstance().getNumCPU();
        for (const auto &pair : processes)
        {
            processSnapshot.push_back(pair.second);
            if (pair.second->getState() == Process::RUNNING) activeCount++;
        }
    }

    std::cout << "CPU utilization: " << (activeCount * 100 / totalCores) << "%\n"
              << "Cores used: " << activeCount << "\n"
              << "Cores available: " << (totalCores - activeCount) << "\n\n";

    std::cout << "Running processes:\n";
    for (const auto &process : processSnapshot)
        if (process->getState() == Process::RUNNING) process->displayProcessInfo();

    std::cout << "\nFinished processes:\n";
    for (const auto &process : processSnapshot)
        if (process->getState() == Process::FINISHED) process->displayProcessInfo();
}

void ProcessManager::startBatch()
{
    if (!Config::getInstance().isInitialized()) throw std::runtime_error("initialize first");

    std::lock_guard<std::mutex> lock(batchMutex);
    if (!batchProcessingActive)
    {
        batchProcessingActive = true;
        batchProcessThread = std::thread(&ProcessManager::batchLoop, this);
    }
}

void ProcessManager::stopBatch()
{
    {
        std::lock_guard<std::mutex> lock(batchMutex);
        if (batchProcessingActive)
        {
            batchProcessingActive = false;
            if (batchProcessThread.joinable()) batchProcessThread.join();
        }
    }
}

void ProcessManager::batchLoop()
{
    int processCounter = 1;
    uint64_t lastCycle = Scheduler::getInstance().getCPUCycles();
    const uint64_t batchFreq = Config::getInstance().getBatchProcessFreq();

    while (batchProcessingActive)
    {
        uint64_t currentCycle = Scheduler::getInstance().getCPUCycles();

        if ((currentCycle - lastCycle) >= batchFreq)
        {
            std::ostringstream processName;
            processName << "p" << std::setfill('0') << std::setw(2) << processCounter++;

            try
            {
                generateProcess(processName.str());
                lastCycle = currentCycle;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error creating batch process: " << e.what() << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

std::string ProcessManager::generateProcessName() const
{
    std::ostringstream oss;
    oss << 'p' << std::setfill('0') << std::setw(2) << nextPID;
    return oss.str();
}
