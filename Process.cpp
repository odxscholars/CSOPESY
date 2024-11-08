#include "headers/Process.h"

Process::Process(std::string processName) : processName(std::move(processName)) {}

void Process::setInstructionsDone(int instructions) {
    instructionsDone = instructions;
}

void Process::setCoreAssigned(int core) {
    coreAssigned = core;
}

void Process::setDone(bool done) {
    isDone = done;
}

void Process::setRunning(bool running) {
    isRunning = running;
}

void Process::setWaiting(bool waiting) {
    isWaiting = waiting;
}

void Process::setEndMemory(int memory) {
}

void Process::setStartMemory(int memory) {
    startMemory = memory;
}

void Process::setInMemory(bool inMemory) {
    isInMemory = inMemory;
}

void Process::setInstructionsTotal(int instructions) {
    instructionsTotal = instructions;
}

std::string Process::getProcessName() const {
    return processName;
}

int Process::getInstructionsDone() const {
    return instructionsDone;
}

int Process::getCoreAssigned() const {
    return coreAssigned;
}

bool Process::getDone() const {
    return isDone;
}

bool Process::getRunning() const {
    return isRunning;
}

bool Process::getWaiting() const {
    return isWaiting;
}

bool Process::getInMemory() const {
    return isInMemory;
}

int Process::getInstructionsTotal() const {
    return instructionsTotal;
}

int Process::getStartMemory() const {
    return startMemory;
}

int Process::getEndMemory() const {
    return endMemory;
}
