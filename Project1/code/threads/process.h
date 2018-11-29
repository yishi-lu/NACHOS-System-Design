#ifndef PROCESS_H
#define PROCESS_H

#include "thread.h"
#include "list.h"
#include "utility.h"
#include "sysdep.h"

//time quantum for all processes, default is 10
static int TIME_QUANTUM = 10;

//process status
enum ProcessStatus { CREATED, IN_RUNNING, IN_READY, IN_BLOCK };

class Process {

	private:
		//process ID
		int processID;
		//process priority
		int priority;
		//execution time of this process, once it reaches time quantum/slice, we need to do context switch
		int executeTime;
		//track child process ID
		int trackCID;
		//current process status
		ProcessStatus status;
		//list for tracking the process's threads, a ready list
		SortedList<Thread *> *threadList;
		//list for tracking the process's child processes
		List<Process *> *children;

		//variables are used to implement Join()
		int finishedChildren;
		int numberOfChildren;
		bool waitingForJoin;

		//variable are used to track thread status (finished or not)
		int finishedThread;
		int numberOfThread;
		bool hasThreadFinished;

		//use to check if current process is scheduled or not
		bool isScheduled;


	public:
		//identify current running thread of the process
		Thread* currentThread;
		//track parent process of current process
		Process* parent;

		//constructor take two parameters ID and priority
		Process(int id, int prior);
		~Process();

		//function used to change time quantum of all processes
		void setTimeQuantum(int q);
		//set parent process of this process
		void setParent(Process *p);
		//return this process ID
		int getPID();
		//set this process status
		void setStatus(ProcessStatus st);
		//set this process priority
		void setPriority(int i);
		//return this process priority
		int getPriority();
		//increment executed time of this process
		void incrementExecutionTime();
		//set execution time of this process
		void setExecutionTime(int i);
		//return execution time of this process
		int getExecutionTime();
		//add child to this process's child process list
		void addChildProcess(Process *p);

		//function for track child processes
		void incrementNumChildren();
		int getNumChildren();
		void incrementFinishedChildren();
		int getFinishedChildren();
		//function for track threds
		void threadFinished();
		int getNumThread();
		int getFinishedThread();

		//indicate the current process is waiting for join or not
		bool getWaitStatus(){return waitingForJoin;};
		//used for Join, when all child processes finished, last will set it to false
		void setWaitStatus(bool st){waitingForJoin = st;}

		//fork new child process
		Process* ForkProcess();
		//fork new thread
		void ForkThread(VoidFunctionPtr func, void *arg);
		//current process given the CPU, context switch happens here
		void Yield();
		//terminate and delete this process
		void Termination();
		//current process sleeps, waiting for all child processer finish
		void Join();
		//block current process until other process interrupt
		void Sleep(bool terminate);
		//print informaton of current process
		void Print();

		//thread scheduler 
		//find next available thread to be run
		Thread* findNextThreadRun();
		//the thread is ready, put it back to the ready list 
		void readyToRun(Thread *t);
		//override "==" operator, so "==" can compare two processes base on their ID
		bool operator==(const Process& p);

		
};

#endif // PROCESS_H