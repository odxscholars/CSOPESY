#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <string>

class CommandManager
{
public:
    enum CommandType
    {
        PRINT
    };

    CommandManager(int pid, CommandType commandType);
    virtual ~CommandManager() = default;

    virtual void execute() = 0;
    virtual std::string getLogDetails() const = 0;

    CommandType getCommandType() const { return commandType; }

protected:
    int pid; // Process ID
    CommandType commandType;
};

#endif