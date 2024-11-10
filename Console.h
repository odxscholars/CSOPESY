#ifndef Console_H
#define Console_H

#include <string>
#include <memory>
#include <windows.h>
#include "Functions.h"

class Console
{
public:
    static Console &getInstance()
    {
        static Console instance;
        return instance;
    }

    void start();
    std::string getCurrentScreen() const { return currentScreen; }

private:
    Console() : initialized(false), currentScreen("main") {}

    bool initialized;
    std::string currentScreen;

    void displayHeader();
    void clearScreen();
    void displayScreenHeader(const std::string &title);
    void displayDivider();
    void handleCommand(const std::string &command);
    void handleScreenCommand(const std::string &flag, const std::string &processName);
    void initialize();
};

#endif