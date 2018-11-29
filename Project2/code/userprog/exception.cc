// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"

//thread call this function to finish
//if it has parent, and the parent is waiting for current thread to finish
//wake up the parent thread
void Exit_POS(int id){

	cout<<"child thread"<<id<<" exits, check parent status."<<endl;
	if(kernel->currentThread->parent != NULL) {
		if(kernel->currentThread->parent->childID == id){
			IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
			cout<<"child thread"<<id<<" wake up parent thread. It finishes"<<endl;
			kernel->scheduler->ReadyToRun(kernel->currentThread->parent);
			kernel->interrupt->SetLevel(oldLevel);
		}
		else if(kernel->currentThread->parent->childID == -1){
				cout<<"ERROR: parent thread is not waiting for any child! Terminat!"<<endl;
				exit(-1);
			 }
		//current thread finish, so remove it from child list of the parent thread
		kernel->currentThread->parent->children->Remove(kernel->currentThread);
	}
	else {
		cout<<"The thread has no parent, it finished!"<<endl;
	}
	kernel->currentThread->Finish();
}

//function copy from project description
void ForkTest1(int id)
{
	printf("ForkTest1 is called, its PID is %d\n", id);
	for (int i = 0; i < 3; i++)
	{
		printf("ForkTest1 is in loop %d\n", i);
		for (int j = 0; j < 100; j++) 
			kernel->interrupt->OneTick();
	}
	Exit_POS(id);
}
//function copy from project description
void ForkTest2(int id)
{
	printf("ForkTest2 is called, its PID is %d\n", id);
	for (int i = 0; i < 3; i++)
	{
		printf("ForkTest2 is in loop %d\n", i);
		for (int j = 0; j < 100; j++) 
			kernel->interrupt->OneTick();
	}
	Exit_POS(id);
}
//function copy from project description
void ForkTest3(int id)
{
	printf("ForkTest3 is called, its PID is %d\n", id);
	for (int i = 0; i < 3; i++)
	{
		printf("ForkTest3 is in loop %d\n", i);
		for (int j = 0; j < 100; j++) 
			kernel->interrupt->OneTick();
	}
	Exit_POS(id);
}


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    //handle page fault exception, allocatePage() will be called to handle it
    case PageFaultException:
    {
        kernel->allocatePage(kernel->machine->ReadRegister(BadVAddrReg));
    }
    case SyscallException:
      switch(type) {
	      case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;

	      case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
			//cout<<"Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n";
			
			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
					/* int op2 */(int)kernel->machine->ReadRegister(5));

			//

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);
			
			/* Modify return point */
			{
			  /* set previous programm counter (debugging only)*/
			  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

			  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			  
			  /* set next programm counter for brach execution */
			  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			}

			return;
			
			ASSERTNOTREACHED();

			break;
		  //handle system call Fork_POS(), new thread will be forked to run some procedure which depend on the input values.
		  case SC_Fork_POS:
		  	{
			  	int i = (int)kernel->machine->ReadRegister(4);

			  	Thread *t = new Thread("FORK_POS");

			  	t->parent = kernel->currentThread;

			  	if(i == 1) {
			  		kernel->currentThread->children->Append(t);
			  		t->threadID = i;
					t->Fork((VoidFunctionPtr) ForkTest1, (void *) 1);
				}
				else if(i == 2) {
					kernel->currentThread->children->Append(t);
					t->threadID = i;
					t->Fork((VoidFunctionPtr) ForkTest2, (void *) 2);
				}
				else if(i == 3) {
					kernel->currentThread->children->Append(t);
					t->threadID = i;
					t->Fork((VoidFunctionPtr) ForkTest3, (void *) 3);
				}
				else {
					cout<<"ERROR: Invalid input! Terminate!"<<endl;
					exit(-1);
				}
				
				kernel->machine->WriteRegister(2, (int)i);
			
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+  4);

			  	return;
			  	ASSERTNOTREACHED();
			  	break;
		    }
		    //handle system call Wait_POS(), child thread ID will be passed in as input
		    //current thread go to sleep until the child is finished
		    //if the child is invalid, then error message will be printed on console
		    //current thread will continue to run 
		   	case SC_Wait_POS:
		  	{
			  	
			  	int child = (int)kernel->machine->ReadRegister(4);

			  	ListIterator<Thread *> *iter = new ListIterator<Thread *>(kernel->currentThread->children);

			  	bool isInList = false;
			  	for (; !iter->IsDone(); iter->Next()) {
					Thread *temp = iter->Item();
					if(temp->threadID == child) {
					    isInList = true;
					    kernel->currentThread->childID = child;
					    break;
					}
				}

				if(!isInList){
					cout<<"ERROR: "<<child<<" is Invalid child ID! Terminate!"<<endl;
					exit(-1);
				}
				else{
				  	IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
				  	cout<<"Parent sleeps and waits for child"<<child<<" finish!"<<endl;
				  	kernel->currentThread->Sleep(false);
				  	kernel->interrupt->SetLevel(oldLevel);

				  	cout<<"Child thread"<<child<<" finishes, parent continue run"<<endl;
				}

				kernel->currentThread->childID = -1;

			  	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+  4);

			  	return;
			  	ASSERTNOTREACHED();
			  	break;
		    }
		    //handle system call Exit(), an integer will be passed in as input
		    //print out some message, and terminate current thread
		    case SC_Exit:
		  	{
			  	int status = (int)kernel->machine->ReadRegister(4);

			  	cout<<"Eixt() userprogram with input "<<status<<endl;

			  	kernel->currentThread->Finish();
			  	
			  	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+  4);

			  	return;
			  	ASSERTNOTREACHED();
			  	break;
		    }
		    //handle system call Write()
		    //char* buffer address is stored in addr, size is stored in size
		    //we call function ReadMem() to read the char* buffer one char at a time
		    //then print the char on the console
		    case SC_Write:
		  	{	
		  		//get input from user program register
			  	int addr = (int)kernel->machine->ReadRegister(4);
			  	int size = (int)kernel->machine->ReadRegister(5);
			  	OpenFileId id = (OpenFileId)kernel->machine->ReadRegister(6);

			  	//set buffer to stored the target char* buffer
			  	int readSize = 0;
			  	char buffer[size+1];
			  	buffer[size] = '\0';

			  	while(readSize < size){
			  		//read targer char* buffer one char by one char
			  		kernel->machine->ReadMem(addr, sizeof(char), (int* )(buffer+readSize)); 

				  	addr+=sizeof(char);
				  	//print the read char to console
				  	printf("%c", buffer[readSize]);

				  	readSize++;

			  	}

			  	cout<<endl;

			  	kernel->machine->WriteRegister(2, (int)size);

			  	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				  
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+  4);

			  	return;
			  	ASSERTNOTREACHED();
			  	break;
		    }


	    default:
			cerr << "Unexpected system call " << type << "\n";
			break;
	   }
	      break;
	    default:
	      cerr << "Unexpected user mode exception" << (int)which << "\n";
	      break;
	}
    ASSERTNOTREACHED();
}
