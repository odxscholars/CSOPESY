//
// Created by shem on 11/6/24.
//

// TODO: [Screen -s] When process is finished and user exits, delete the process
#include <iostream>
#include <set>
#include <vector>
#ifndef PROCESS_H
#define PROCESS_H
#include <ctime>
class Process {
public:
  explicit Process(std::string processName);
  void setInstructionsDone(int instructions);
  void setCoreAssigned(int core);

  void setDone(bool done);
  void setRunning(bool running);
  void setWaiting(bool waiting);
  void setScreenName(std::string screenName);
  void setProcessSize(int processSize);

  void setInstructionsTotal(int instructions); // only called once
  // getters
  std::string getProcessName() const;
  int getInstructionsDone() const;
  int getCoreAssigned() const;
  int getProcessSize() const;

  bool getDone() const;
  bool getRunning() const;
  bool getWaiting() const;

  int getInstructionsTotal() const;
  std::string getScreenName() const;

  std::time_t startTime = 0;
  std::time_t endTime = 0;

  // paging stuff
  std::vector<int> pages; // pages that the process is using
    bool isRunning = false;

private:
  std::string processName;
  std::string screenName;
  int processSize;
  int coreAssigned = -1;

  // BOOLEAN FLAGS
  bool isDone = false;    // when not waiting to be done by scheduler
   // when running on a core
  bool isWaiting = false; // in ready queue
                          // in main memory for scheduling

  int instructionsDone = 0;
  int instructionsTotal = 0;
};
#endif // PROCESS_H
