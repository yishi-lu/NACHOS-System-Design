#include "process.h"
#include "synch.h"
#include "sysdep.h"

//compare two process by their priority, used when insert process into ready list
static int ThreadCompare (Thread *x, Thread *y)  
{  
    return x->getOrder() - y->getOrder();
} 

//constructor, initializing basic variable of process. id and priority need be given
Process::Process(int id, int prior){
	processID = id;
	priority = prior;
	trackCID = 0;
	executeTime = 0;
	parent = NULL;
	waitingForJoin = false;

	finishedThread = 0;
	numberOfThread = 0;
	hasThreadFinished = false;
	isScheduled = false;

	numberOfChildren = 0;
	finishedChildren = 0;

	threadList = new SortedList<Thread *>(ThreadCompare);
	children = new List<Process *>;

	currentThread = new Thread("dummy thread");//dummy thread of new process
}

//deconstructor, delete two list and all threads
Process::~Process(){

	while(!threadList->IsEmpty()) delete threadList->RemoveFront();

	delete threadList;

	delete children;
}

//set time Quantum
void Process::setTimeQuantum(int q){
	TIME_QUANTUM = q;
	cout<<"Time quantum is set to: "<<TIME_QUANTUM<<endl;
	return;
}

void Process::setParent(Process *p){
	parent = p;
}

int Process::getPID(){
	return processID;
}

void Process::setPriority(int i){
	priority = i;
}

int Process::getPriority(){
	return priority;
}

void Process::incrementExecutionTime(){
	executeTime++;
}

void Process::setExecutionTime(int i){
	executeTime = i;
}

int Process::getExecutionTime(){
	return executeTime;
}

void Process::setStatus(ProcessStatus st){
	status = st;
}

void Process::addChildProcess(Process *p){
	children->Append(p);
}

void Process::incrementNumChildren(){
	numberOfChildren++;
}

int Process::getNumChildren(){
	return numberOfChildren;
}

void Process::incrementFinishedChildren(){
	finishedChildren++;
}

int Process::getFinishedChildren(){
	return finishedChildren;
}

void Process::threadFinished(){
	finishedThread++;
	hasThreadFinished = true;
}

int Process::getNumThread(){
	return numberOfThread;
}

int Process::getFinishedThread(){
	return finishedThread;
}

//fork new child process
Process* Process::ForkProcess(){

	Process *child = new Process(this->getPID()*100+this->trackCID, this->getPriority());
	trackCID++;

	child->setParent(this);

	incrementNumChildren();

	ListIterator<Thread *> *iter = new ListIterator<Thread *>(this->threadList);

	cout<<"Process"<<processID<<" forks new Process. child process with ID:"<<child->getPID()<<" has priority "<<child->getPriority()<<endl;

	//get func and arg information of parent threads, and use these information to fork new threads
	for (; !iter->IsDone(); iter->Next()) {
		Thread* t = iter->Item();
		child->ForkThread(t->fork_func, (void*) t->fork_arg);
	}

	return child;
}

//fork new thread
void Process::ForkThread(VoidFunctionPtr func, void *arg){
	
	Interrupt *interrupt = kernel->interrupt;
    Scheduler *scheduler = kernel->scheduler;
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);

 	numberOfThread++;

 	cout<<"Process"<<processID<<" forks new thread, it has "<<numberOfThread<<" threads"<<endl;

	Thread *t = new Thread("thread");

	//set thread arrive order
	t->setOrder(numberOfThread);

	//allocate space for new thread
	t->Fork(func, arg);

	if(threadList->IsEmpty()) currentThread = t;

	readyToRun(t);

	//if it is the first thread of the process, we need schedule the process in kernel, so the process can run
	//if it is not, we do not need to schedule it again because it is already scheduled
	if(!isScheduled) {
		isScheduled = true;
		kernel->scheduler->ReadyToRun(this);
	}
	
}

//check if context switch is needed
void Process::Yield ()
{
    Process *nextProcess;
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    
    ASSERT(this == kernel->currentProcess);

    nextProcess = kernel->scheduler->getNextProcess();

    //there is more than one process
    if (nextProcess != NULL) {
    	//if there is heigher priority process, 
    	//or current heighest priority process used all time quantum (there are processes with same priority as current process)
    	//then we do context switch to another process 
    	if(waitingForJoin || nextProcess->getPriority()>this->getPriority() || (this->getExecutionTime() >= TIME_QUANTUM && nextProcess->getPriority()==this->getPriority())) { 
    		 cout<<"-----Context Switch between TWO PROCESSES-----"<<endl;
    		 this->setExecutionTime(0);  
    		 kernel->scheduler->FindNextToRun();   
             if(!waitingForJoin) kernel->scheduler->ReadyToRun(this);  
             kernel->scheduler->Run(nextProcess, FALSE);
        }
        //current process has heighest priority, so we still run the current process. 
        //context switch is not needed 
        else {
        	if(this->getExecutionTime() >= TIME_QUANTUM){
        		cout<<"-----Current Process use all its time quantum, but it has highest priority, so still running it-----"<<endl;
        		this->setExecutionTime(0);  
        	}
        	//if current running thread is finished, we switch to another thread
        	if(hasThreadFinished){
        		cout<<"-----One thread of the process is finished, switch to another thread-----"<<endl;
        		hasThreadFinished = false;
        		kernel->scheduler->Run(this, FALSE);
        	}
        }
    }
    //there is only one process
    else{
    	if(this->getExecutionTime() >= TIME_QUANTUM){
        	cout<<"-----Current Process use all its time quantum, it is the only ready process, so still running it-----"<<endl;
        	this->setExecutionTime(0);  
        }
        //if current running thread is finished, we switch to another thread
        if(hasThreadFinished){
        	cout<<"-----One thread of the process is finished, switch to another thread-----"<<endl;
        	hasThreadFinished = false;
        	kernel->scheduler->Run(this, FALSE);
        }
    }

    (void) kernel->interrupt->SetLevel(oldLevel);
}

//delete all thread of the process, and finally delete the process
void Process::Termination(){

	//before the process terminate, it check if its parent can run
	//if yes put the parent process in ready list
	cout<<">>>>>Process"<<this->processID<<" terminates<<<<<"<<endl;
	if(parent != NULL && parent->getWaitStatus()) {
		parent->incrementFinishedChildren();
		if(parent->getFinishedChildren() == parent->getNumChildren()) {
			cout<<"All child processes are finished, wake up parent Process"<<parent->getPID()<<"."<<endl;
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			kernel->scheduler->ReadyToRun(parent);
			parent->setExecutionTime(-1);  
			parent->setWaitStatus(false);
			(void) kernel->interrupt->SetLevel(oldLevel);
		}
	}
 
	Sleep(true);
}

//current process will be blocked
void Process::Join(){
	IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
	status = IN_BLOCK;
	waitingForJoin = true;
	cout<<"Parent Process"<<processID<<" call Join(), it is blocked, and it waits for child processes to finish."<<endl;
	(void) kernel->interrupt->SetLevel(oldLevel);
	//Sleep(false);
}

//process is blocked. If terminate is ture, then the process will be deleted
void Process::Sleep(bool terminate){

	Process *nextProcess;

	status = IN_BLOCK;

	while ((nextProcess = kernel->scheduler->FindNextToRun()) == NULL) kernel->interrupt->Idle();

	nextProcess->setExecutionTime(-1);

	kernel->scheduler->Run(nextProcess, terminate); 

}

//find next thread to run
Thread* Process::findNextThreadRun(){
	ASSERT(kernel->interrupt->getLevel() == IntOff);
	if(threadList->IsEmpty()) return NULL;
	return threadList->RemoveFront();
}

//insert a thread to thread ready list
void Process::readyToRun(Thread *t){
	ASSERT(kernel->interrupt->getLevel() == IntOff);
	t->setStatus(READY);
    threadList->Insert(t);
}

void Process::Print(){
	IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
	cout<<"Process"<<processID<<" has priority:"<<priority<<endl;
	(void) kernel->interrupt->SetLevel(oldLevel);
}

//compare two processes by "==" operator
bool Process::operator==(const Process& p) { return processID == p.processID; }

