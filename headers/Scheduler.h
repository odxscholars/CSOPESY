#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Config.h"
#include "Process.h"
#include "MemoryManager.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
struct Core {
    std::thread* thread;
    Process* process;
    bool isRunning = false;
    bool isIdle = false;


};
class Scheduler {
public:
    Scheduler(Config config, std::vector<Process*>* processVector);
    void addProcessToReadyQueue(Process* process);
    void startSchedulerTest();
    std::vector<Core>* getCoreVector();

private:
    void generateDummyProcesses();
    void runFCFSScheduler(int cpuIndex);
    void runRR(int cpuIndex);
    void startThreads();
    void taskManager();

    int numCores;
    std::string schedulingAlgorithm;
    int quantumCycles;
    int batchProcessFrequency;
    int minimumInstructions;
    int maxInstructions;
    int delaysPerExecution;
    int globalExecDelay;
    bool schedulerTestRunning = false;
    int processCounter = 0;

    std::vector<Core> coreVector;
    std::vector<Process*>* processVector;
    std::queue<Process*> readyQueue;
    std::vector<Process*> finishedProcesses;
    std::mutex mtx;
    std::mutex memoryManagerMutex;
    std::condition_variable cv;
    std::thread generateThread;
    MemoryManager memoryManager;
};

#endif // SCHEDULER_H