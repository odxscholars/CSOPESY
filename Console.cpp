#include "Console.h"
#include "Config.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <windows.h>

void Console::displayHeader() {
        std::cout << R"(
                               _____  _____  ____  _____  ______  _______     __
                              / ____|/ ____|/ __ \|  __ \|  ____|/ ____\ \   / /
                             | |    | (___ | |  | | |__) | |__  | (___  \ \_/ /
                             | |     \___ \| |  | |  ___/|  __|  \___ \  \   /
                             | |____ ____) | |__| | |    | |____ ____) |  | |
                              \_____|_____/ \____/|_|    |______|_____/   |_|
    )";
    std::cout << "                                                   \n";
    std::cout << "____________________________________________________________________________________________________________\n\n";
    std::cout << "                                          Command Line Emulator\n";
    std::cout << "\n____________________________________________________________________________________________________________\n\n";

    std::cout << "\033[32mHello, Welcome to CSOPESY command line interface!\n\n"
              << "\033[0m";

    std::cout << "\n\nList of Available Commands:\n\n";
    std::cout << "  'initialize'     - Initialize something\n";
    std::cout << "  'screen'         - Manage screen operations\n";
    std::cout << "  'scheduler-test' - Test the scheduler\n";
    std::cout << "  'scheduler-stop' - Stop the scheduler\n";
    std::cout << "  'report-util'    - Run report utility\n";
    std::cout << "  'marquee'        - Start the marquee application\n";
    std::cout << "  'clear'          - Clear the screen\n";
    std::cout << "  'exit'           - Exit the application\n";
}

void Console::start()
{
    displayHeader();
    std::string input;

    while (true)
    {
        if (currentScreen == "main")
        {
            std::cout << "\nroot\\> ";
        }
        else
        {
            std::cout << "\n"
                      << currentScreen << "\\> ";
        }

        std::getline(std::cin, input);

        if (input == "exit")
        {
            if (currentScreen != "main")
            {
                clearScreen();
                currentScreen = "main";
                displayHeader();
                continue;
            }
            break;
        }

        try
        {
            handleCommand(input);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

void Console::handleCommand(const std::string &command)
{
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (!initialized && cmd != "initialize" && cmd != "exit")
    {
        std::cout << "initialize first.\n";
        return;
    }

    if (currentScreen == "main")
    {
        if (cmd == "initialize")
        {
            initialize();
        }
        else if (cmd == "screen")
        {
            std::string screenCommand;
            std::string processName;
            iss >> screenCommand >> processName;

            if (screenCommand == "-s" || screenCommand == "-r" || screenCommand == "-ls")
            {
                handleScreenCommand(screenCommand, processName);
            }
            else
            {
                std::cout << "Invalid screen command. Use -s <name>, -r <name>, or -ls\n";
            }
        }
        else if (cmd == "scheduler-test")
        {
            Functions::getInstance().startBatch();
            std::cout << "Batch process generation started.\n";
        }
        else if (cmd == "scheduler-stop")
        {
            Functions::getInstance().stopBatch();
            std::cout << "Batch process generation stopped.\n";
        }
        else if (cmd == "report-util")
        {
            Scheduler::getInstance().cpuUtil();
        }
        else if (cmd == "clear"){
            clearScreen();
            displayHeader();
        }
        else if (cmd == "marquee")
        {
            std::cout << "Starting marquee application...\n";
            std::cout << "Press Ctrl+C to stop.\n";
            while (true)
            {
                std::cout << "CSOPESY ";
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                std::cout << "\b\b\b\b\b\b\b";
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
        else if (cmd != "exit")
        {
            std::cout << "Invalid command.\n";
        }
    }
    else
    {
        if (cmd == "process-smi")
        {
            auto process = Functions::getInstance().getProcess(currentScreen);
            if (process)
            {
                process->displayProcessInfo();
            }
            else
            {
                std::cout << "Process does not exist.\n";
                currentScreen = "main";
                clearScreen();
                displayHeader();
            }
        }
        else if (cmd != "exit")
        {
            std::cout << "Invalid command. Available commands: process-smi, exit\n";
        }
    }
}

void Console::initialize()
{
    Config::getInstance().loadConfig("config.txt");
    initialized = true;
    Scheduler::getInstance().startScheduling();
    Config::getInstance().displayConfig();


}

void Console::handleScreenCommand(const std::string &screenCommand, const std::string &processName)
{
    if (screenCommand == "-s")
    {
        // Screen creation command
        if (processName.empty())
        {
            std::cout << "Error: Process name is required\n";
            return;
        }

        try
        {
            Functions::getInstance().generateProcess(processName);
            clearScreen();
            currentScreen = processName;

            displayScreenHeader("Process Screen: " + processName);

            std::cout << "Available Commands:\n"
                      << "  [1] process-smi   - Show process information\n"
                      << "  [2] exit          - Return to main menu\n\n";

            displayDivider();

            auto process = Functions::getInstance().getProcess(processName);
            if (process)
            {
                process->displayProcessInfo();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in screen command: " << e.what() << "\n";
            currentScreen = "main";
        }
    }
    else if (screenCommand == "-r")
    {
        auto process = Functions::getInstance().getProcess(processName);
        if (process && process->getState() != Process::FINISHED)
        {
            clearScreen();
            currentScreen = processName;

            displayScreenHeader("Resuming Process Screen: " + processName);

            std::cout << "Commands:\n"
                      << "  [1] process-smi   - Show process information\n"
                      << "  [2] exit          - Return to main menu\n\n";

            displayDivider();
            process->displayProcessInfo();
        }
        else
        {
            std::cout << "Process \"" << processName << "\" not found or has finished.\n";
        }
    }
    else if (screenCommand == "-ls")
    {
        std::cout << "\n=========================================\n";
        std::cout << "          Active Processes List          \n";
        std::cout << "=========================================\n";
        
        Functions::getInstance().listProcesses();
        
        std::cout << "=========================================\n\n";
    }
}

void Console::displayScreenHeader(const std::string &title)
{
    std::cout << "\n=========================================\n";
    std::cout << "          " << title << "\n";
    std::cout << "=========================================\n";
}

void Console::displayDivider()
{
    std::cout << "-----------------------------------------\n\n";
}


void Console::displayProcessScreen(const std::string &processName)
{
    clearScreen();

    std::cout << "╔════════════════════════════════════════╗\n"
              << "║           PROCESS SCREEN               ║\n"
              << "╠════════════════════════════════════════╣\n"
              << "║ Process: " << std::setw(28) << std::left << processName << "║\n"
              << "╠════════════════════════════════════════╣\n"
              << "║ Available Commands:                    ║\n"
              << "║  • process-smi - Show process info     ║\n"
              << "║  • exit        - Return to main menu   ║\n"
              << "╚════════════════════════════════════════╝\n\n";

    if (auto process = Functions::getInstance().getProcess(processName))
    {
        std::cout << "Process Details:\n";
        std::cout << "────────────────────────────────────────\n";
        process->displayProcessInfo();
        std::cout << "────────────────────────────────────────\n";
    }
    else
    {
        std::cout << "Error: Process \"" << processName << "\" not found.\n";
    }
}


void Console::clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}