#include "headers/Scheduler.h"
#include "headers/Process.h"
#include <chrono>
#include <cstdlib>
// #include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

Scheduler::Scheduler(Config config, std::vector<Process *> *processVector)
    : memoryManager(config.getMaxOverallMemory(), config.getMemoryPerFrame(),
                    config.getMemoryPerProcess()) {
  numCores = config.getNumCpu();
  schedulingAlgorithm = config.getScheduler();
  quantumCycles = config.getQuantumCycles();
  batchProcessFrequency = config.getBatchProcessFreq();
  minimumInstructions = config.getMinIns();
  maxInstructions = config.getMaxIns();
  delaysPerExecution = config.getDelaysPerExec();
  this->processVector = processVector;
  this->globalExecDelay = (delaysPerExecution + 1) * 100;

  coreVector.resize(numCores);
  // Initialize the coreVector
  for (int i = 0; i < numCores; ++i) {
    coreVector[i].coreIndex = i;
    coreVector[i].state = CoreState::IDLE;
    coreVector[i].process = nullptr;
    coreVector[i].thread = nullptr;
  }
}

void Scheduler::addProcessToReadyQueue(Process *process) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    readyQueue.push(process);
  }
  cv.notify_one(); // Notify outside the lock
}

void Scheduler::addCustomProcess(Process *process) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    readyQueue.push(process);
    processVector->push_back(process);
  }
  cv.notify_one();
}

void Scheduler::generateDummyProcesses() {
  while (schedulerTestRunning) {
    int generatedInstructions =
        rand() % (maxInstructions - minimumInstructions + 1) +
        minimumInstructions;
    auto *newProcess = new Process("Process_" + std::to_string(processCounter));
    newProcess->setInstructionsTotal(generatedInstructions);
    newProcess->setWaiting(true);
    {
      std::lock_guard<std::mutex> lock(mtx);
      processVector->push_back(newProcess);
    }

    addProcessToReadyQueue(newProcess);

    processCounter++;
    std::this_thread::sleep_for(
        std::chrono::milliseconds(batchProcessFrequency));
  }
}

void Scheduler::runFCFSScheduler(int cpuIndex) {
  while (schedulerTestRunning || !readyQueue.empty()) {
    Process *currentProcess;
    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock,
              [this] { return !readyQueue.empty() || !schedulerTestRunning; });

      if (!schedulerTestRunning && readyQueue.empty()) {
        return;
      }

      currentProcess = readyQueue.front();
      readyQueue.pop();
    }

    coreVector[cpuIndex].process = currentProcess;
    coreVector[cpuIndex].state = CoreState::RUNNING;

    currentProcess->setCoreAssigned(cpuIndex);
    currentProcess->setRunning(true);
    currentProcess->setWaiting(false);

    int instructions = currentProcess->getInstructionsTotal();

    currentProcess->startTime = std::time(nullptr);

    for (int i = 0; i < instructions; ++i) {
      currentProcess->setInstructionsDone(i + 1);
      std::this_thread::sleep_for(
          std::chrono::milliseconds(delaysPerExecution));
    }
    currentProcess->endTime = std::time(nullptr);

    currentProcess->setRunning(false);
    currentProcess->setDone(true);
    coreVector[cpuIndex].process = nullptr;
    coreVector[cpuIndex].state = CoreState::IDLE;

    finishedProcesses.push_back(currentProcess);
  }
}
void Scheduler::runRR(int cpuIndex) {
  Process *currentProcess = nullptr;
  while (threadsContinue) {
    if (currentProcess == nullptr) {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this] { return !readyQueue.empty() || !threadsContinue; });

      if (!threadsContinue && readyQueue.empty()) {
        return;
      }

      currentProcess = readyQueue.front();
      if (currentProcess == nullptr) { // null check
        continue;
      }

      // std::cout << "Next Process: " << currentProcess->getProcessName() <<
      // std::endl;
      readyQueue.pop();
    }

    {
      std::lock_guard<std::mutex> lock(memoryManagerMutex);
      if (!memoryManager.isProcessInMemory(currentProcess->getProcessName()) &&
          !memoryManager.allocateMemory(currentProcess->getProcessName(),
                                        memoryManager.memoryPerProcess)) {
        addProcessToReadyQueue(currentProcess);
        currentProcess = nullptr;
        continue;
      }
    }
    // std::cout << "passed checks" << std::endl;
    coreVector[cpuIndex].process = currentProcess;
    coreVector[cpuIndex].state = CoreState::RUNNING;

    currentProcess->setCoreAssigned(cpuIndex);
    currentProcess->setRunning(true);
    currentProcess->setWaiting(false);

    int instructions = currentProcess->getInstructionsTotal();
    int executedInstructions = currentProcess->getInstructionsDone();
    int remainingInstructions = instructions - executedInstructions;
    int quantum = std::min(quantumCycles, remainingInstructions);

    if (currentProcess->startTime == 0) {
      currentProcess->startTime = std::time(nullptr);
    }

    for (int i = 0; i < quantum; ++i) {
      std::stringstream formatName;

      formatName << "memory_stamp_" << quantumCycles << ".txt";

      currentProcess->setInstructionsDone(executedInstructions + i + 1);
      memoryManager.generateReport(formatName.str());
      std::this_thread::sleep_for(std::chrono::milliseconds(globalExecDelay));
    }

    if (currentProcess->getInstructionsDone() == instructions) {
      currentProcess->endTime = std::time(nullptr);
      currentProcess->setRunning(false);
      currentProcess->setDone(true);
      coreVector[cpuIndex].process = nullptr;
      coreVector[cpuIndex].state = CoreState::IDLE;
      memoryManager.deallocateMemory(currentProcess->getProcessName());
      finishedProcesses.push_back(currentProcess);
      currentProcess = nullptr;
    } else {
      if (!readyQueue.empty()) {
        currentProcess->setRunning(false);
        currentProcess->setWaiting(true);
        coreVector[cpuIndex].process = nullptr;
        coreVector[cpuIndex].state = CoreState::IDLE;
        addProcessToReadyQueue(currentProcess);

        currentProcess = nullptr;
      }
    }
  }
}

void Scheduler::startThreads() {
  // Start the thread for generating dummy processes
  generateThread = std::thread(&Scheduler::generateDummyProcesses, this);
  generateThread.detach();
  std::thread taskManagerThread(&Scheduler::taskManager, this);
  taskManagerThread.detach();
}

void Scheduler::taskManager() {
  while (schedulerTestRunning) {
    {
      this->memoryManager.VisualizeMemory();
      std::cout << "Ready Queue:" << std::endl;
      // print ready queue
      std::queue<Process *> tempQueue = readyQueue;
      while (!tempQueue.empty()) {
        std::cout << tempQueue.front()->getProcessName() << std::endl;
        tempQueue.pop();
      }
      // print a divider
      std::cout << "----------------" << std::endl;
      for (int i = 0; i < numCores; ++i) {
        if (coreVector[i].process != nullptr) {
          std::cout << "CPU " << i << " "
                    << coreVector[i].process->getProcessName() << " "
                    << std::put_time(
                           std::localtime(&coreVector[i].process->startTime),
                           "%Y-%m-%d %H:%M:%S")
                    << " " << coreVector[i].process->getInstructionsDone()
                    << "/" << coreVector[i].process->getInstructionsTotal()
                    << std::endl;
        } else {
          // print idle
          std::cout << "CPU " << i << " Idle" << std::endl;
        }
      }
      std::cout << "----------------" << std::endl;
      std::cout << "Finished Processes:" << std::endl;
      for (auto &process : finishedProcesses) {
        std::cout << process->getProcessName() << " "
                  << process->getInstructionsDone() << "/"
                  << process->getInstructionsTotal() << std::endl;
      }
      std::cout << "----------------" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(globalExecDelay));
  }
}

void Scheduler::stopSchedulerTest() { schedulerTestRunning = false; }

void Scheduler::bootStrapthreads() {
  threadsContinue = true;
  // start the scheduler thread
  for (int i = 0; i < numCores; ++i) {
    if (schedulingAlgorithm == "fcfs") {
      coreVector[i].thread =
          new std::thread(&Scheduler::runFCFSScheduler, this, i);
    } else if (schedulingAlgorithm == "rr") {
      coreVector[i].thread = new std::thread(&Scheduler::runRR, this, i);
    }
    coreVector[i].thread->detach();
  }
}

std::vector<Core> *Scheduler::getCoreVector() { return &coreVector; }

void Scheduler::startSchedulerTest() {
  std::cout << "Starting Scheduler Test" << std::endl;
  schedulerTestRunning = true;
  startThreads();
}
