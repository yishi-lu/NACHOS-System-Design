
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "process.h"


//the scheduler now only schedule Processes, not Thread
class Scheduler {
  public:
    Scheduler();	 
    ~Scheduler();		

    void ReadyToRun(Process* process);	
    				
    Process* FindNextToRun();	
				
    void Run(Process* nextProcess, bool finishing);
    				
    void CheckToBeDestroyed();

    //new function, it retrun the first process in the ready list without remove it
    Process* getNextProcess();

    
  private:
    //a sorted list to keep track ready process
    //we insert process base on their priority from large to small
    SortedList<Process *> *readyList;  
				
    Process *toBeDestroyed;	
};

#endif // SCHEDULER_H
