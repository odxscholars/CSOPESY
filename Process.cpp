#include "headers/Process.h"
#include "headers/PrintCommand.h"
#include "headers/Utils.h"

#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>

Process::Process(int pid, const std::string &name)
    : pid(pid),
      name(name),
      state(READY),
      cpuCoreID(-1),
      commandCounter(0),
      quantumTime(0),
      creationTime(std::chrono::system_clock::now())
{
    int numInstructions = generateInstructionCount();
    commandList.resize(numInstructions, std::make_shared<PrintCommand>(pid, name));
}

void Process::executeCurrentCommand(int coreID)
{
    if (commandCounter < commandList.size())
    {
        try
        {
            commandList[commandCounter++]->execute();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error executing command in process " << pid << ": " << e.what() << std::endl;
        }
    }
}

bool Process::isFinished()
{
    return commandCounter >= commandList.size();
}

int Process::generateInstructionCount() const
{
    auto &config = Config::getInstance();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(config.getMinInstructions(), config.getMaxInstructions());
    return dis(gen);
}

void Process::displayProcessInfo()
{
    std::lock_guard<std::mutex> lock(processMutex);
    std::cout << name << " (" << formatTimestamp(creationTime) << ") ";
    std::cout << (state == FINISHED ? "Finished   " : "Core: " + std::to_string(cpuCoreID) + "    ")
              << commandCounter << " / " << commandList.size() << "\n";
}

int Process::getPID() const { return pid; }
std::string Process::getName() const { return name; }
Process::ProcessState Process::getState() { return state.load(); }
void Process::setState(ProcessState newState) { state.store(newState); }
void Process::setCPUCoreID(int id) { cpuCoreID.store(id); }
int Process::getCPUCoreID() { return cpuCoreID.load(); }
int Process::getCommandCounter() { return commandCounter.load(); }
int Process::getLinesOfCode() { return static_cast<int>(commandList.size()); }
