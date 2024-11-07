#include "headers/Scheduler.h"
#include "headers/Process.h"
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>

Scheduler::Scheduler(Config config, std::vector<Process>* processVector) {
    numCores = config.getNumCpu();
    schedulingAlgorithm = config.getScheduler();
    quantumCycles = config.getQuantumCycles();
    batchProcessFrequency = config.getBatchProcessFreq();
    minimumInstructions = config.getMinIns();
    maxInstructions = config.getMaxIns();
    delaysPerExecution = config.getDelaysPerExec();
    this->processVector = processVector;

    cpuStatus.resize(numCores, "");
    currentInstructions.resize(numCores, 0);
    totalInstructions.resize(numCores, 0);
    coreVector.resize(numCores);
    // Initialize the coreVector
    for (int i = 0; i < numCores; ++i) {
        coreVector[i].isIdle = true;
        coreVector[i].isRunning = false;
        coreVector[i].process = nullptr;
        coreVector[i].thread = nullptr;
    }
}

void Scheduler::addProcessToReadyQueue(const Process &process) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        readyQueue.push(process);
    }
    cv.notify_one(); // Notify outside the lock
}

void Scheduler::generateDummyProcesses() {
    while (schedulerTestRunning) {

        int generatedInstructions = rand() % (maxInstructions - minimumInstructions + 1) + minimumInstructions;
        Process newProcess("Process " + std::to_string(processCounter));
        newProcess.setInstructionsTotal(generatedInstructions);
        newProcess.setWaiting(true);

        {
            std::lock_guard<std::mutex> lock(mtx);
            processVector->push_back(newProcess);
        }

        Process &processInVector = processVector->back();

        addProcessToReadyQueue(processInVector);


        processCounter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(batchProcessFrequency));
    }
}

void Scheduler::runFCFSScheduler(int cpuIndex) {
    while (schedulerTestRunning || !readyQueue.empty()) {
        Process currentProcess("");
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !readyQueue.empty() || !schedulerTestRunning; });

            if (!schedulerTestRunning && readyQueue.empty()) {
                return;
            }

            currentProcess = readyQueue.front();
            readyQueue.pop();
        }

        coreVector[cpuIndex].process = &currentProcess;
        currentProcess.setCoreAssigned(cpuIndex);
        currentProcess.setWaiting(false);
        currentProcess.setRunning(true);

        int instructions = currentProcess.getInstructionsTotal();
        totalInstructions[cpuIndex] = instructions;
        currentInstructions[cpuIndex] = 0;

        // FCFS scheduling logic
        for (int i = 0; i < instructions; ++i) {
            currentInstructions[cpuIndex] = i + 1;
            currentProcess.setInstructionsDone(i + 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(delaysPerExecution));
        }

        std::cout << "CPU " << cpuIndex << " processed " << currentProcess.getProcessName() << " for " << instructions <<
                  " instructions.\n";

        cpuStatus[cpuIndex] = "";
        currentInstructions[cpuIndex] = 0;
        totalInstructions[cpuIndex] = 0;

        finishedProcesses.push_back(currentProcess.getProcessName());
    }
}

void Scheduler::startThreads() {
    // Start the thread for generating dummy processes
    generateThread = std::thread(&Scheduler::generateDummyProcesses, this);
    generateThread.detach();

    //start the core threads from coreVector
    for (int i = 0; i < numCores; ++i) {
        coreVector[i].thread = new std::thread(&Scheduler::runFCFSScheduler, this, i);
        coreVector[i].thread->detach();
    }

    //start the task manager thread
    std::thread taskManagerThread(&Scheduler::taskManager, this);
    taskManagerThread.detach();


}

void Scheduler::taskManager() {
    while (schedulerTestRunning) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Ready Queue: ";
            std::queue<Process> tempQueue = readyQueue;
            while (!tempQueue.empty()) {
                std::cout << tempQueue.front().getProcessName() << " ";
                tempQueue.pop();
            }
            std::cout << std::endl;

            for (int i = 0; i < numCores; ++i) {
                std::cout << "Core " << i << ": ";
                if (cpuStatus[i].empty()) {
                    std::cout << "Idle";
                } else {
                    std::cout << cpuStatus[i] << " ("
                              << currentInstructions[i] << "/"
                              << totalInstructions[i] << ")";
                }
                std::cout << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

void Scheduler::startSchedulerTest() {
    std::cout << "Starting Scheduler Test" << std::endl;
    schedulerTestRunning = true;
    startThreads();
}