#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <map>
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include "headers/Process.h"
#include "Scheduler.h"

class ProcessManager
{
public:
    static ProcessManager &getInstance()
    {
        static ProcessManager instance;
        return instance;
    }

    void generateProcess(const std::string &name);
    std::shared_ptr<Process> getProcess(const std::string &name);
    void listProcesses();
    void startBatch();
    void stopBatch();

private:
    ProcessManager() : nextPID(1), batchProcessingActive(false), lastProcessCreationCycle(0) {}
    ~ProcessManager() { stopBatch(); }

    std::map<std::string, std::shared_ptr<Process>> processes;
    std::atomic<int> nextPID;
    std::atomic<bool> batchProcessingActive;
    std::thread batchProcessThread;
    std::mutex processesMutex;
    std::mutex batchMutex;
    uint64_t lastProcessCreationCycle;

    void batchLoop();
    std::string generateProcessName() const;
};

#endif