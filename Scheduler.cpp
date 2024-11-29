#include "headers/Scheduler.h"
#include "headers/MemoryManager.h"
#include "headers/Process.h"
#include <chrono>
#include <cstdlib>
// #include <format>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

Scheduler::Scheduler(Config config, std::vector<Process *> *processVector)
    : config(config),
      memoryManager(config.getMaxOverallMemory(), config.getMemoryPerFrame(),
                    config.getMinMemoryPerProcess(),
                    config.getMaxMemoryPerProcess(),
                    config.getMemoryPerFrame()) {
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

    int memorySize = rand() % (config.getMaxMemoryPerProcess() -
                               config.getMinMemoryPerProcess() + 1) +
                     config.getMinMemoryPerProcess();
    auto *newProcess = new Process("Process_" + std::to_string(processCounter));
    newProcess->setInstructionsTotal(generatedInstructions);
    newProcess->setProcessSize(memorySize);
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
      if (currentProcess == nullptr) {
        // null check
        continue;
      }

      // std::cout << "Next Process: " << currentProcess->getProcessName() <<
      // std::endl;
      readyQueue.pop();
    }
    {
      std::lock_guard<std::mutex> lock(memoryManagerMutex);
      // TODO: I think `memoryManager.minMemoryPerProcess` is not correct.
      // Should be referring to Process memorySize
      if (!memoryManager.isProcessInMemory(currentProcess->getProcessName()) &&
          !memoryManager.allocateMemory(currentProcess->getProcessName(),
                                        currentProcess->getProcessSize())) {
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
      currentProcess->setInstructionsDone(executedInstructions + i + 1);
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

void Scheduler::runPagingRR(int cpuIndex) {
  Process *currentProcess = nullptr;
  while (threadsContinue) {
    if (currentProcess == nullptr) {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this] { return !readyQueue.empty() || !threadsContinue; });

      if (!threadsContinue && readyQueue.empty()) {
        return;
      }

      currentProcess = readyQueue.front();
      if (currentProcess == nullptr) {
        // null check
        continue;
      }
      readyQueue.pop();
    }
    if (currentProcess != nullptr) {
      // check if process is in memory
      if (!memoryManager.isProcessinPagingMemory(currentProcess)) {
        // if not in memory, allocate memory
        int processPageReq =
            std::ceil(static_cast<double>(currentProcess->getProcessSize()) /
                      memoryManager.frameSize);
        std::lock_guard<std::mutex> lock(allocateMemoryMutex);
        if (!memoryManager.pagingAllocate(currentProcess, processPageReq)) {
          // if memory allocation fails, add process to ready queue and continue
          addProcessToReadyQueue(currentProcess);
          currentProcess = nullptr;
          continue;
        }
      }
      // std::lock_guard<std::mutex> lock(allocateMemoryMutex);
      // // CASE 1: If the process is not in memory and memory allocation fails
      // because memory is full int processPageReq = std::ceil(
      //     static_cast<double>(currentProcess->getProcessSize()) /
      //     memoryManager.frameSize);
      // if (!memoryManager.isProcessinPagingMemory(currentProcess) &&
      //     !memoryManager.pagingAllocate(currentProcess, processPageReq)) {
      //     // IF memory allocation fails and not in memory, de-allocate oldest
      //     process and allocate current process
      //     addProcessToReadyQueue(currentProcess);
      //     currentProcess = nullptr;
      //
      //     continue;
      // }
    }
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
      currentProcess->setInstructionsDone(executedInstructions + i + 1);
      std::this_thread::sleep_for(std::chrono::milliseconds(globalExecDelay));
    }

    if (currentProcess->getInstructionsDone() == instructions) {
      currentProcess->endTime = std::time(nullptr);
      currentProcess->setRunning(false);
      currentProcess->setDone(true);
      coreVector[cpuIndex].process = nullptr;
      coreVector[cpuIndex].state = CoreState::IDLE;
      {
        std::lock_guard<std::mutex> lock(deallocateMemoryMutex);
        memoryManager.pagingDeallocate(currentProcess);
      }

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
  /*std::thread taskManagerThread(&Scheduler::taskManager, this);*/
  std::thread reportGenerator(&Scheduler::generateReportPerCycle, this);
  /*taskManagerThread.detach();*/
  reportGenerator.detach();
}

void Scheduler::taskManager() {
  while (schedulerTestRunning) {
    {
      std::cout << "Ready Queue:" << std::endl;
      // print ready queue
      std::queue<Process *> tempQueue = readyQueue;
      while (!tempQueue.empty()) {
        // std::cout << tempQueue.front()->getProcessName() << std::endl;
        tempQueue.pop();
      }
      this->memoryManager.visualizeFrames();
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
      int count = 0;
      for (auto it = finishedProcesses.rbegin();
           it != finishedProcesses.rend() && count < 5; ++it, ++count) {
        std::cout << (*it)->getProcessName() << " "
                  << (*it)->getInstructionsDone() << "/"
                  << (*it)->getInstructionsTotal() << std::endl;
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
      if (config.getMaxOverallMemory() == config.getMemoryPerFrame()) {
        coreVector[i].thread = new std::thread(&Scheduler::runRR, this, i);
      } else {
        std::cout << "Paging RR" << std::endl;
        coreVector[i].thread =
            new std::thread(&Scheduler::runPagingRR, this, i);
      }
    }
    coreVector[i].thread->detach();
  }
}

void Scheduler::generateReportPerCycle() {
  int rr = 0;
  while (schedulerTestRunning) {
    for (const auto &core : coreVector) {
      if (core.state == RUNNING) {
        activeTicks += 1;
      } else {
        idleTicks += 1;
      }
    }
    std::stringstream formatName;
    formatName << "memory_stamp_" << rr << ".txt";
    {
      std::lock_guard<std::mutex> lock(memoryManagerMutex);
      memoryManager.generateReport(formatName.str());
    }
    rr += quantumCycles;
    std::this_thread::sleep_for(std::chrono::milliseconds(globalExecDelay));
  }
}

std::vector<Core> *Scheduler::getCoreVector() { return &coreVector; }

void Scheduler::startSchedulerTest() {
  std::cout << "Starting Scheduler Test" << std::endl;
  schedulerTestRunning = true;
  startThreads();
}

MemoryManager Scheduler::getMemoryManager() { return memoryManager; }
