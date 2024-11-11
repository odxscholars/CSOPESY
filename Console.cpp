#include "headers/Console.h"
#include "headers/Process.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// #include "ScreenCommand.h"
#include "headers/Config.h"
#include "headers/Scheduler.h"
#include "headers/Marquee.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
// TODO: Implement report-util
// TODO: Implement Screen commands handling
// TODO: Implement process-smi
// TODO: Implement memory manager

void Console::startConsole() {
  displayMainMenu();
  int cpuCycles = 0;
  // Get the input command for main menu
  std::string command;
  bool session = false;
  while (true) {
    if (session) {
      std::cout << "\nroot :\\> ";
    } else {
      std::cout << "\nEnter command: ";
    }
    std::getline(std::cin, command);
    processCommand(command, session);
    cpuCycles++;
  }
}

bool isInitialized = false;

void Console::clearScreen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}
void Console::displayMainMenu() {
  std::cout << R"(
                               _____  _____  ____  _____  ______  _______     __
                              / ____|/ ____|/ __ \|  __ \|  ____|/ ____\ \   / /
                             | |    | (___ | |  | | |__) | |__  | (___  \ \_/ /
                             | |     \___ \| |  | |  ___/|  __|  \___ \  \   /
                             | |____ ____) | |__| | |    | |____ ____) |  | |
                              \_____|_____/ \____/|_|    |______|_____/   |_|
    )";
  std::cout << "                                                   \n";
  std::cout << "_______________________________________________________________"
               "_____________________________________________\n\n";
  std::cout
      << "                                          Command Line Emulator\n";
  std::cout << "\n_____________________________________________________________"
               "_______________________________________________\n\n";

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

void Console::processSMI(Process &process) {
  std::cout << "Process: " << process.getScreenName() << "\n"
            << "ID: " << process.getProcessName() << "\n\n"
            << "Current instruction lines: " << process.getInstructionsDone()
            << "\n"
            << "Lines of code: " << process.getInstructionsTotal();
}

std::string Console::generateReport() {
  std::ostringstream report;
  int cpuUtil = 0;
  int coreUsed = 0;
  int coreAvailable = 0;

  for (const auto &core : *coreVector) {
    if (core.isIdle) {
      coreAvailable++;
    } else {
      coreUsed++;
    }
  }
  cpuUtil = (coreUsed / coreCount) * 100;

  report << "CPU utiliziation: " << cpuUtil << "%";
  report << "\nCores used: " << coreUsed;
  report << "\nCores available: " << coreAvailable << "\n";
  report << "-----------------------------------------------------------\n";

  /*report << "Existing custom processes: \n";*/
  /*for (const std::string &screen : existingSessions) {*/
  /*  report << numbering << ": " << screen << "\n";*/
  /*  numbering++;*/
  /*}*/
  report << "\n\nRunning Processes: \n";
  // for (const auto &process : *processVector) {
  //   if (process->getRunning() == true) {
  //     report << process->getProcessName() << "\t("
  //            << std::put_time(std::localtime(&process->startTime),
  //                             "%Y-%m-%d %H:%M:%S")
  //            << ")\t Core: " << process->getCoreAssigned() << "\t "
  //            << process->getInstructionsDone() << "/"
  //            << process->getInstructionsTotal() << "\n";
  //   }
  // }
  //iterate through the coreVector
  for (const auto &core : *coreVector) {
    if (core.isRunning) {
      report << core.process->getProcessName() << "\t("
             << std::put_time(std::localtime(&core.process->startTime),
                              "%Y-%m-%d %H:%M:%S")
             << ")\t Core: " << core.process->getCoreAssigned() << "\t "
             << core.process->getInstructionsDone() << "/"
             << core.process->getInstructionsTotal() << "\n";
    }else {
      //print "CPU {} Idle"
      report << "CPU " << core.coreIndex << " Idle\n";

    }
  }

  report << "\n\nFinished Processes: \n";
  for (const auto &process : *processVector) {
    if (process->getDone() == true) {
      report << process->getProcessName() << "\t ("
             << std::put_time(std::localtime(&process->startTime),
                              "%Y-%m-%d %H:%M:%S")
             << ")\t Status: Finished"
             << "\t " << process->getInstructionsDone() << "/"
             << process->getInstructionsTotal() << "\n";
    }
  }
  // }
  report << "-----------------------------------------------------------\n";
  return report.str();
}

void Console::processCommand(const std::string &command, bool &session) {
  std::stringstream ss(command);
  std::string cmd, option, screenName;
  Process *currentProcess;
  ss >> cmd >> option >> screenName;

  // bool started = false;

  // static Scheduler* scheduler = nullptr;

  if (!isInitialized && command != "initialize" && command != "exit") {
    std::cout << "Initialize First.\n";
    return;
  }

  if (command == "initialize") {
    Config loadedConfig("config.txt");
    loadedConfig.loadConfig();
    scheduler = new Scheduler(
        loadedConfig, processVector); // IMPORTANT: ProcessVector is a pointer
                                      // to the processVector in Console
    maxins = loadedConfig.getMaxIns();
    minins = loadedConfig.getMinIns();
    coreCount = loadedConfig.getNumCpu();
    loadedConfig.displayConfig();
    isInitialized = true;
    std::cout << "Initialized using config.txt\n";
    this->coreVector =
        scheduler->getCoreVector(); // IMPORTANT: This is a pointer to the
                                    // coreVector in Scheduler
    scheduler->bootStrapthreads();
  } else if (session) {
    if (cmd == "process-smi") {
      if (currentProcess->getDone() == false) {
        processSMI(*currentProcess);
      } else {
        std::cout << "Finished!";
      }
    } else if (cmd == "exit") {
      session = false;
      currentProcess = nullptr;
      sessionName = "";
      clearScreen();
    } else {
      std::cout << "Uknown command. Please try again!";
    }
  } else if (cmd == "screen") {
    // ScreenCommand screenCommand(scheduler);

    if (option == "-ls") {
      std::cout << generateReport();
    } else if (option == "-s") {
      if (screenName != "") {
        auto item = std::find(existingSessions.begin(), existingSessions.end(),
                              screenName);
        if (item == existingSessions.end()) {
          session = true;
          sessionName = screenName;
          existingSessions.push_back(sessionName);

          Process *newProcess = new Process(screenName);
          currentProcess = newProcess;
          newProcess->setScreenName(screenName);
          newProcess->setInstructionsTotal(rand() % (maxins - minins + 1) +
                                           minins);
          scheduler->addCustomProcess(newProcess);
          clearScreen();
          processSMI(*currentProcess);
        } else {
          std::cout << "Custom Process already exists";
        }
      }
    } else if (option == "-r") {

      auto item = std::find(existingSessions.begin(), existingSessions.end(),
                            screenName);
      if (item == existingSessions.end()) {
        std::cout << "No custom process exists!";
      } else {
        for (const auto &process : *processVector) {
          if (screenName == process->getScreenName()) {
            session = true;
            currentProcess = process;
            sessionName = process->getScreenName();
            clearScreen();
            processSMI(*currentProcess);
            break;
          }
        }
      }
      // screenCommand.processScreenCommand(option, screenName);
    }
  } else if (command == "scheduler-test") {
    scheduler->startSchedulerTest();
    // if (scheduler) {
    //     // scheduler->startSchedulerTest();
    // } else {
    //     std::cout << "Scheduler not initialized.\n";
    // }
  } else if (command == "scheduler-stop") {
    scheduler->stopSchedulerTest();
    // if (scheduler) {
    //     scheduler->stopSchedulerTest();
    // } else {
    //     std::cout << "Scheduler not initialized.\n";
    // }
  } else if (command == "report-util") {
    // std::filesystem::path current_dir = std::filesystem::current_path();
    std::ofstream outputFile("csopesy-log.txt");
    std::cout << "Running report utility...\n";

    if (outputFile.is_open()) {
      outputFile << generateReport();
      std::cout << "Report successfully generated!";
    } else {
      std::cout << "Something went wrong while opening the file!!" << std::endl;
    }


  } else if (command == "marquee") {
    Marquee marquee;
    marquee.startMarquee();
  } else if (command == "clear") {
    clearScreen();
    displayMainMenu();
  } else if (command == "exit") {
    std::cout << "Exiting the application.\n";
    // if (scheduler) {
    //     delete scheduler;
    // }
    exit(0);
  } else {
    std::cout << "Unknown command. Try again.\n";
  }
}
