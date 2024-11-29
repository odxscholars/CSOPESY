#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Config.h"
#include "MemoryManager.h"
#include "Process.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
enum CoreState { RUNNING, IDLE, STOPPED };
struct Core {
  int coreIndex;
  std::thread *thread;
  Process *process;
  CoreState state;
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

  void runPagingRR(int cpuIndex);

  void startThreads();

  void taskManager();

  void stopSchedulerTest();

  void bootStrapthreads();

  void generateReportPerCycle();

  MemoryManager *getMemoryManager();

  std::vector<Core> *getCoreVector();

  int globalExecDelay = 0;
  Config config;
  std::condition_variable memoryCv;
  int totalTicks = 0;
  int idleTicks = 0;
  int activeTicks = 0;

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
  int cycles;

  bool threadsContinue = false;
  bool schedulerTestRunning = false;
  int processCounter = 0;

  std::vector<Process *> finishedProcesses;
  std::mutex mtx;
  std::mutex memoryManagerMutex;
  std::mutex allocateMemoryMutex;
  std::mutex deallocateMemoryMutex;

  std::condition_variable cv;
  std::thread generateThread;
  MemoryManager memoryManager;
};

#endif // SCHEDULER_H
