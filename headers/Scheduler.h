#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Process.h"
#include "Config.h"

class Scheduler {
public:
    Scheduler(Config config, std::vector<Process>* processVector);

    void addProcessToReadyQueue(const Process &process);
    void generateDummyProcesses();
    void startSchedulerTest();
    void runFCFSScheduler(int cpuIndex);
    void startThreads();

    void taskManager();

private:
    std::queue<Process> readyQueue;
    std::vector<Process>* processVector;
    int numCores;
    std::string schedulingAlgorithm;
    int quantumCycles;
    int batchProcessFrequency;
    int minimumInstructions;
    int maxInstructions;
    int delaysPerExecution;

    bool schedulerTestRunning = false;
    int processCounter = 0;

    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> threadVector;
    std::thread generateThread;

    std::vector<std::string> cpuStatus;
    std::vector<int> currentInstructions;
    std::vector<int> totalInstructions;
    std::vector<std::string> finishedProcesses;
};

#endif //SCHEDULER_H