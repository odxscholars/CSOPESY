//
// Created by shem on 11/6/24.
//

// TODO: [Screen -s] When process is finished and user exits, delete the process
#include<iostream>
#ifndef PROCESS_H
#define PROCESS_H
#include<ctime>
class Process {
public:
    explicit Process(std::string processName);
    void setInstructionsDone(int instructions);
    void setCoreAssigned(int core);

    void setDone(bool done);
    void setRunning( bool running);
    void setWaiting( bool waiting);
    void setProcessNameID(int id);


    void setInstructionsTotal(int instructions); //only called once
    // getters
    std::string getProcessName() const;
    int getInstructionsDone() const;
    int getCoreAssigned() const;
    bool getDone() const;
    bool getRunning() const;
    bool getWaiting() const;
    int getInstructionsTotal() const;
    int getProcessNameID() const;


    std::time_t startTime = 0 ;
    std::time_t endTime = 0;





private:


    int processNameID;
    
    std::string processName;
    int coreAssigned = -1;
    bool isDone = false; //when not waiting to be done by scheduler
    bool isRunning = false; //when running on a core
    bool isWaiting = false; // in ready queue

    int instructionsDone = 0;
    int instructionsTotal = 0;

};
#endif //PROCESS_H
