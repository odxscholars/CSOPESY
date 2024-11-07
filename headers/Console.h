//
// Created by shem on 11/6/24.
//

#ifndef CONSOLE_H
#define CONSOLE_H
#include <vector>
#include "Process.h"
#include <string>

#include "Scheduler.h"

class Console {
    public:
        void startConsole();
        void clearScreen();
        void displayMainMenu();
        void processCommand(const std::string &command);
        std::vector<Process*> * processVector = new std::vector<Process*>;
        Scheduler* scheduler;

private:






};

#endif //CONSOLE_H




