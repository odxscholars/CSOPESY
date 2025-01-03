//
// Created by shem on 11/6/24.
//

#ifndef CONSOLE_H
#define CONSOLE_H
/*#include <bemapiset.h>*/

#include "Process.h"
#include <string>
#include <vector>

#include "Scheduler.h"

class Console {
public:
  void startConsole();
  void clearScreen();
  void displayMainMenu();
  void processSMI(Process &process);
  void processCommand(const std::string &command, bool &session);

  std::string generateReport();
  std::vector<Process *> *processVector = new std::vector<Process *>;
  std::vector<Core> *coreVector;
  Scheduler *scheduler;

private:
  Process *currentSessionProcess = nullptr;
  std::vector<std::string> existingSessions;
  int maxins, minins, coreCount;
  std::string sessionName;
  int maxOverallMem, memPerFrame;
};

#endif // CONSOLE_H
