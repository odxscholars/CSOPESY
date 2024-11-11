#include "headers/Scheduler.h"
#include "headers/TimeStamps.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <ctime>



Scheduler::Scheduler()
{
    size_t numCPUs = Config::getInstance().getNumCPU();
    coreStatus.resize(numCPUs, false);

    //week 8
    size_t memPerProc = Config::getInstance().getMemPerProc();
    size_t maxOverallMem = Config::getInstance().getMaxOverallMem();
    size_t memPerFrame = Config::getInstance().getMemPerFrame();
}

void Scheduler::startScheduling()
{
    processingActive = true;
    isInitialized = true;
    cpuCycles.store(0);

    int numCPUs = Config::getInstance().getNumCPU();
    cpuThreads.reserve(numCPUs);

    try {
        for (int i = 0; i < numCPUs; ++i)
            cpuThreads.emplace_back(&Scheduler::processHandler, this);
    } catch (const std::exception& e) {
        std::cerr << "Error initializing CPU threads: " << e.what() << std::endl;
        processingActive = isInitialized = false;
        return;
    }

    cycleCounter = true;
    cycleThreadCount = std::thread(&Scheduler::counter, this);

    std::cout << "Scheduler successfully started with " << numCPUs << " CPUs." << std::endl;
}

void Scheduler::stopScheduleProcess()
{
    processingActive = cycleCounter = false;
    cv.notify_all();
    syncCv.notify_all();

    for (auto &thread : cpuThreads) {
        if (thread.joinable())
            thread.join();
    }
    cpuThreads.clear();

    if (cycleThreadCount.joinable())
        cycleThreadCount.join();
}


void Scheduler::addProcess(std::shared_ptr<Process> process)
{
    if (!process) return;

    Config& config = Config::getInstance();
    uint16_t memoryPerProcess = config.getMemPerProc();
    uint16_t availableMemory = config.getMaxOverallMem();

    std::unique_lock<std::timed_mutex> lock(mutex, std::defer_lock);

    if (lock.try_lock_for(std::chrono::milliseconds(100))) {
        // Check if enough memory is available
        if (memoryPerProcess <= availableMemory) {
            // Allocate memory for the process
            availableMemory -= memoryPerProcess;
            
            // Add the process to the ready queue
            readyQueue.push(process);
            cv.notify_all();
        } else {
            // Not enough memory, re-add to the end of the ready queue
            readyQueue.push(process);
        }
        
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


void Scheduler::processHandler()
{
    Config& config = Config::getInstance();
    uint16_t memoryPerProcess = config.getMemPerProc();
    uint16_t availableMemory = config.getMaxOverallMem();
    
    while (processingActive)
    {
        std::shared_ptr<Process> currentProcess = nullptr;
        bool hasProcess = false;

        {
            std::unique_lock<std::timed_mutex> lock(mutex);
            hasProcess = !readyQueue.empty() || cv.wait_for(lock, std::chrono::milliseconds(100), [this]
                                                             { return !processingActive || !readyQueue.empty(); });
            if (!processingActive) break;
            if (hasProcess && !readyQueue.empty()) currentProcess = nextProcess();
        }

        if (currentProcess)
        {
            currentProcess->setState(Process::RUNNING);
            int delays = config.getDelaysPerExec(), currentDelay = 0;

            while (!currentProcess->isFinished() && processingActive)
            {
                if (config.getSchedulerType() == "rr" &&
                    currentProcess->getQuantumTime() >= config.getQuantumCycles())
                {
                    updateCoreStatus(currentProcess->getCPUCoreID(), false);
                    quantumHandler(currentProcess);
                    break;
                }

                if (currentDelay < delays)
                {
                    currentDelay++;
                }
                else
                {
                    currentProcess->executeCurrentCommand(currentProcess->getCPUCoreID());
                    currentProcess->moveToNextLine();
                    currentDelay = 0;
                    if (config.getSchedulerType() == "rr") currentProcess->incrementQuantumTime();
                }

                synchronization();
            }

            {
                std::lock_guard<std::timed_mutex> lock(mutex);
                if (currentProcess->isFinished())
                {
                    currentProcess->setState(Process::FINISHED);
                    finishedProcesses.push_back(currentProcess);

                    // Release memory for the finished process
                    availableMemory += memoryPerProcess;

                    updateCoreStatus(currentProcess->getCPUCoreID(), false);
                }
                else if (config.getSchedulerType() != "rr")
                {
                    currentProcess->setState(Process::READY);
                    readyQueue.push(currentProcess);
                }
                runningProcesses.erase(std::remove(runningProcesses.begin(), runningProcesses.end(), currentProcess), runningProcesses.end());
            }

            cv.notify_all();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            cv.notify_all();
        }
    }
}



std::shared_ptr<Process> Scheduler::nextProcess()
{
    if (readyQueue.empty()) return nullptr;

    int availableCore = std::distance(coreStatus.begin(), std::find(coreStatus.begin(), coreStatus.end(), false));
    if (availableCore == coreStatus.size()) return nullptr;

    std::shared_ptr<Process> nextProcess = (Config::getInstance().getSchedulerType() == "rr") ? RR() : FCFS();

    if (nextProcess)
    {
        nextProcess->setCPUCoreID(availableCore);
        coreStatus[availableCore] = true;
        runningProcesses.push_back(nextProcess);
    }

    return nextProcess;
}


std::shared_ptr<Process> Scheduler::FCFS()
{
    if (readyQueue.empty()) return nullptr;
    auto process = readyQueue.front();
    readyQueue.pop();
    return process;
}

std::shared_ptr<Process> Scheduler::RR()
{
    if (readyQueue.empty()) return nullptr;

    auto process = readyQueue.front();
    readyQueue.pop();

    if (quantumExpiration(process))
    {
        quantumHandler(process);
        if (readyQueue.empty()) return nullptr;
        
        process = readyQueue.front();
        readyQueue.pop();
    }

    return process;
}

void Scheduler::generateMemorySnapshot(int quantumCycle) {
    std::stringstream filename;
    filename << "memory_stamp_" << quantumCycle << ".txt";
    
    std::ofstream file(filename.str());
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename.str() << " for writing." << std::endl;
        return;
    }

    // Add the timestamp
    auto timestamp = formatTimestamp(std::chrono::system_clock::now());
    file << "TimeStamp: (" << timestamp << ")\n";
    
    // Collect memory information
    int numProcesses = runningProcesses.size();  // Number of processes currently running
    int totalMemory = Config::getInstance().getMaxOverallMem();
    int memPerProcess = Config::getInstance().getMemPerProc();
    int externalFragmentation = 0;

    // Calculate fragmentation and prepare ASCII layout
    int memoryUsed = 0;
    file << "Number of processes in memory: " << numProcesses << "\n";
    
    for (const auto& process : runningProcesses) {
        int upperLimit = memoryUsed + memPerProcess;
        file << upperLimit << "\nP" << process->getPID() << "\n" << memoryUsed << "\n";
        memoryUsed = upperLimit;
    }

    externalFragmentation = totalMemory - memoryUsed;
    file << "Total external fragmentation in KB: " << externalFragmentation << "\n";
    
    // Print memory layout boundaries
    file << "----end---- = " << totalMemory << "\n";
    file << "----start---- = 0\n";
    
    file.close();
    std::cout << "Memory snapshot created: " << filename.str() << std::endl;
}


bool Scheduler::quantumExpiration(const std::shared_ptr<Process> &process) const
{
    return process->getQuantumTime() >= Config::getInstance().getQuantumCycles();
}

void Scheduler::quantumHandler(std::shared_ptr<Process> process)
{
    process->resetQuantumTime();
    process->setState(Process::READY);
    readyQueue.push(process);
}

void Scheduler::cpuUtil() const //report utilization also
{
    std::stringstream report;
    int totalCores, usedCores;
    std::vector<std::shared_ptr<Process>> runningProcessesCopy, finishedProcessesCopy;

    {
        std::lock_guard<std::timed_mutex> lock(mutex);
        totalCores = Config::getInstance().getNumCPU();
        usedCores = runningProcesses.size();
        runningProcessesCopy = runningProcesses;
        finishedProcessesCopy = finishedProcesses;
    }

    report << "CPU utilization: " << (usedCores * 100 / totalCores) << "%\n"
           << "Cores used: " << usedCores << "\n"
           << "Cores available: " << (totalCores - usedCores) << "\n\n"
           << "Running processes:\n";

    auto timestamp = formatTimestamp(std::chrono::system_clock::now());
    for (const auto &process : runningProcessesCopy)
    {
        report << process->getName() << " (" << timestamp << ")   "
               << "Core: " << process->getCPUCoreID() << "    "
               << process->getCommandCounter() << " / " << process->getLinesOfCode() << "\n";
    }

    report << "\nFinished processes:\n";
    for (const auto &process : finishedProcessesCopy)
    {
        report << process->getName() << " (" << timestamp << ")   "
               << "Finished    " << process->getLinesOfCode() << " / " << process->getLinesOfCode() << "\n";
    }

    std::cout << report.str();

    std::ofstream logFile("nathanbaho.txt", std::ios::app);
    if (logFile)
    {
        logFile << report.str() << "\n";
        logFile.close();
        std::cout << "nathanbaho.txt created!\n";
    }
}


void Scheduler::synchronization()
{
    const int CYCLE_SPEED = 1000;
    const int CYCLE_WAIT = 500;

    std::unique_lock<std::timed_mutex> syncLock(syncMutex);
    std::this_thread::sleep_for(std::chrono::microseconds(CYCLE_SPEED));

    int runningCount;
    {
        std::lock_guard<std::timed_mutex> lock(mutex);
        runningCount = runningProcesses.size();
    }

    if (runningCount == 0)
    {
        incrementCPUCycles();
        std::this_thread::sleep_for(std::chrono::microseconds(CYCLE_WAIT));
        return;
    }

    coresWaiting++;
    if (coresWaiting >= runningCount)
    {
        incrementCPUCycles();
        coresWaiting = 0;
        syncCv.notify_all();
        std::this_thread::sleep_for(std::chrono::microseconds(CYCLE_WAIT));
    }
    else
    {
        syncCv.wait_for(syncLock, std::chrono::microseconds(CYCLE_WAIT), [this] { return coresWaiting == 0; });
        std::this_thread::sleep_for(std::chrono::microseconds(CYCLE_SPEED));

        if (coresWaiting > 0)
        {
            coresWaiting = 0;
            incrementCPUCycles();
            syncCv.notify_all();
        }
    }
}


void Scheduler::updateCoreStatus(int coreID, bool active)
{
    if (coreID >= 0 && coreID < static_cast<int>(coreStatus.size()))
    {
        coreStatus[coreID] = active;
        if (!active) cv.notify_all();
    }
}

void Scheduler::counter()
{
    while (cycleCounter)
    {
        {
            std::unique_lock<std::timed_mutex> lock(syncMutex);
            if (runningProcesses.empty() && readyQueue.empty())
            {
                incrementCPUCycles();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
}

