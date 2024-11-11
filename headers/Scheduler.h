#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Config.h"
#include "Process.h"
#include "MemoryManager.h"
#include <vector>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Process.h"
#include "Config.h"
struct Core {
    std::thread* thread;
    Process* process;
    bool isRunning = false;
    bool isIdle = false;


};
class Scheduler {
public:
  Scheduler(Config config, std::vector<Process *> *processVector);

  void addProcessToReadyQueue(Process *process);
  void addCustomProcess(Process *process);
  void generateDummyProcesses();
  void startSchedulerTest();
  void runFCFSScheduler(int cpuIndex);

  void runRR(int cpuIndex);

  void startThreads();

  void taskManager();

  void stopSchedulerTest();

  void bootStrapthreads();

  std::vector<Core> *getCoreVector();

  int globalExecDelay = 0;

private:
  std::queue<Process *> readyQueue;
  std::vector<Process *> *processVector;
  std::vector<Core> coreVector;
  int numCores;
  std::string schedulingAlgorithm;
  int quantumCycles;
  int batchProcessFrequency;
  int minimumInstructions;
  int maxInstructions;
  int delaysPerExecution;

  bool schedulerTestRunning = false;
  int processCounter = 0;

    std::vector<Process*> finishedProcesses;
    std::mutex mtx;
    std::mutex memoryManagerMutex;
    std::condition_variable cv;
    std::thread generateThread;
    MemoryManager memoryManager;
};

#endif //SCHEDULER_H