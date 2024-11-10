#include "headers/LogPrints.h"
#include "headers/Console.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

LogPrints::LogPrints(int pid, const std::string &processName)
    : CommandManager(pid, CommandType::PRINT), processName(processName) {}

std::string LogPrints::getLogDetails() const
{
    return "Nathan di naliligo " + processName;
}


void LogPrints::execute()
{
    // Empty implementation to satisfy linker
}

