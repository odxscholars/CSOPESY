#ifndef PRINTCOMMAND_H
#define PRINTCOMMAND_H

#include "CommandManager.h"
#include <string>
#include <mutex>

class PrintCommand : public CommandManager
{
public:
    // PrintCommand(int pid, const std::string &toPrint);
    PrintCommand(int pid, const std::string &processName);

    void execute() override;
    std::string getLogDetails() const override;

private:
    // std::string toPrint;
    std::string processName;
};

#endif