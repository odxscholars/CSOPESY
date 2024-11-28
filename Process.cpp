#include "headers/Process.h"

Process::Process(std::string processName)
    : processName(std::move(processName)) {}

void Process::setInstructionsDone(int instructions) {
  instructionsDone = instructions;
}

void Process::setCoreAssigned(int core) { coreAssigned = core; }

void Process::setProcessSize(int processSize) {
  this->processSize = processSize;
}

void Process::setDone(bool done) { isDone = done; }

void Process::setScreenName(std::string screenName) {
  this->screenName = screenName;
}
void Process::setRunning(bool running) { isRunning = running; }

void Process::setWaiting(bool waiting) { isWaiting = waiting; }

void Process::setInstructionsTotal(int instructions) {
  instructionsTotal = instructions;
}

std::string Process::getProcessName() const { return processName; }

int Process::getInstructionsDone() const { return instructionsDone; }

int Process::getCoreAssigned() const { return coreAssigned; }

int Process::getProcessSize() const { return processSize; }

bool Process::getDone() const { return isDone; }

bool Process::getRunning() const { return isRunning; }

bool Process::getWaiting() const { return isWaiting; }

std::string Process::getScreenName() const { return screenName; }

int Process::getInstructionsTotal() const { return instructionsTotal; }
