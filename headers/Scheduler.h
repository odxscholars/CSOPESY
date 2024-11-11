#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include "Process.h"
#include "Config.h"

class Scheduler
{
public:
    static Scheduler &getInstance()
    {
        static Scheduler instance;
        return instance;
    }

    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler &) = delete;

    void addProcess(std::shared_ptr<Process> process);
    void startScheduling();
    void stopScheduleProcess();
    void cpuUtil() const;
    uint64_t getCPUCycles() const { return cpuCycles.load(); }

private:
    Scheduler();
    ~Scheduler() { stopScheduleProcess(); }

    std::atomic<bool> isInitialized{false};


    std::queue<std::shared_ptr<Process>> readyQueue;
    std::vector<std::shared_ptr<Process>> runningProcesses;
    std::vector<std::shared_ptr<Process>> finishedProcesses;
    mutable std::timed_mutex mutex;
    mutable std::timed_mutex syncMutex;
    std::condition_variable_any cv;
    std::condition_variable_any syncCv;
    std::atomic<int> coresWaiting{0};
    std::atomic<bool> processingActive{false};
    std::vector<std::thread> cpuThreads;
    std::vector<bool> coreStatus;
    std::atomic<uint64_t> cpuCycles{0};
    std::thread cycleThreadCount;
    std::atomic<bool> cycleCounter{false};
    void processHandler();
    std::shared_ptr<Process> nextProcess();
    std::shared_ptr<Process> RR();
    std::shared_ptr<Process> FCFS();
    void quantumHandler(std::shared_ptr<Process> process);
    bool quantumExpiration(const std::shared_ptr<Process> &process) const;
    void updateCoreStatus(int coreID, bool active);
    void synchronization();
    void counter();

    // week 8 additions
    void generateMemorySnapshot(int quantumCycle);
    
    void incrementCPUCycles() { 
        ++cpuCycles; 
    }
};

#endif
