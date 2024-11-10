#ifndef LOGPRINTS_H
#define LOGPRINTS_H

#include "CommandManager.h"
#include <string>
#include <mutex>

class LogPrints : public CommandManager
{
public:
    LogPrints(int pid, const std::string &processName);

    void execute() override;
    std::string getLogDetails() const override;

private:
    std::string processName;
};

#endif

