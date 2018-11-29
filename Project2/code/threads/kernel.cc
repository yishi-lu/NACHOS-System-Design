// kernel.cc 
//	Initialization and cleanup routines for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "main.h"
#include "kernel.h"
#include "sysdep.h"
#include "synch.h"
#include "synchlist.h"
#include "libtest.h"
#include "string.h"
#include "synchconsole.h"
#include "synchdisk.h"
#include "post.h"

//----------------------------------------------------------------------
// Kernel::Kernel
// 	Interpret command line arguments in order to determine flags 
//	for the initialization (see also comments in main.cc)  
//----------------------------------------------------------------------

Kernel::Kernel(int argc, char **argv)
{
    randomSlice = FALSE; 
    debugUserProg = FALSE;
    consoleIn = NULL;          // default is stdin
    consoleOut = NULL;         // default is stdout
#ifndef FILESYS_STUB
    formatFlag = FALSE;
#endif
    reliability = 1;            // network reliability, default is 1.0
    hostName = 0;               // machine id, also UNIX socket name
                                // 0 is the default machine id
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-rs") == 0) {
 	    ASSERT(i + 1 < argc);
	    RandomInit(atoi(argv[i + 1]));// initialize pseudo-random
					// number generator
	    randomSlice = TRUE;
	    i++;
        } else if (strcmp(argv[i], "-s") == 0) {
            debugUserProg = TRUE;
	} else if (strcmp(argv[i], "-ci") == 0) {
	    ASSERT(i + 1 < argc);
	    consoleIn = argv[i + 1];
	    i++;
	} else if (strcmp(argv[i], "-co") == 0) {
	    ASSERT(i + 1 < argc);
	    consoleOut = argv[i + 1];
	    i++;
#ifndef FILESYS_STUB
	} else if (strcmp(argv[i], "-f") == 0) {
	    formatFlag = TRUE;
#endif
        } else if (strcmp(argv[i], "-n") == 0) {
            ASSERT(i + 1 < argc);   // next argument is float
            reliability = atof(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-m") == 0) {
            ASSERT(i + 1 < argc);   // next argument is int
            hostName = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-u") == 0) {
            cout << "Partial usage: nachos [-rs randomSeed]\n";
	    cout << "Partial usage: nachos [-s]\n";
            cout << "Partial usage: nachos [-ci consoleIn] [-co consoleOut]\n";
#ifndef FILESYS_STUB
	    cout << "Partial usage: nachos [-nf]\n";
#endif
            cout << "Partial usage: nachos [-n #] [-m #]\n";
	}
    }
}

//----------------------------------------------------------------------
// Kernel::Initialize
// 	Initialize Nachos global data structures.  Separate from the 
//	constructor because some of these refer to earlier initialized
//	data via the "kernel" global variable.
//----------------------------------------------------------------------

void
Kernel::Initialize()
{
    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main");		
    currentThread->setStatus(RUNNING);

    stats = new Statistics();		// collect statistics
    interrupt = new Interrupt;		// start up interrupt handling
    scheduler = new Scheduler();	// initialize the ready queue
    alarm = new Alarm(randomSlice);	// start up time slicing
    machine = new Machine(debugUserProg);
    synchConsoleIn = new SynchConsoleInput(consoleIn); // input from stdin
    synchConsoleOut = new SynchConsoleOutput(consoleOut); // output to stdout
    synchDisk = new SynchDisk();    //


    PPT = new PhysicalPageEntry[NumPhysPages];
    //initialize physical page table and bitmap
    memoryManager = new Bitmap(NumPhysPages);
    //initialize physical page entry
    for (int i = 0; i < NumPhysPages; i++) {
        PPT[i].virtualPage = -1;
        PPT[i].pageOrder = 0;
        PPT[i].threadOnThisPage = NULL;
    }

#ifdef FILESYS_STUB
    fileSystem = new FileSystem();
#else
    fileSystem = new FileSystem(formatFlag);
#endif // FILESYS_STUB
    postOfficeIn = new PostOfficeInput(10);
    postOfficeOut = new PostOfficeOutput(reliability);

    interrupt->Enable();
}

//----------------------------------------------------------------------
// Kernel::~Kernel
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------

Kernel::~Kernel()
{
    delete stats;
    delete interrupt;
    delete scheduler;
    delete alarm;
    delete machine;
    delete synchConsoleIn;
    delete synchConsoleOut;
    delete synchDisk;
    delete fileSystem;
    delete postOfficeIn;
    delete postOfficeOut;
    delete PPT;
    delete memoryManager;
    
    Exit(0);
}

//----------------------------------------------------------------------
// Kernel::ThreadSelfTest
//      Test threads, semaphores, synchlists
//----------------------------------------------------------------------

void
Kernel::ThreadSelfTest() {
   Semaphore *semaphore;
   SynchList<int> *synchList;
   
   LibSelfTest();		// test library routines
   currentThread->SelfTest();	// test thread switching
   
   				// test semaphore operation
   semaphore = new Semaphore("test", 0);
   semaphore->SelfTest();
   delete semaphore;
   
   				// test locks, condition variables
				// using synchronized lists
   synchList = new SynchList<int>;
   synchList->SelfTest(9);
   delete synchList;

}

//----------------------------------------------------------------------
// Kernel::ConsoleTest
//      Test the synchconsole
//----------------------------------------------------------------------

void
Kernel::ConsoleTest() {
    char ch;

    cout << "Testing the console device.\n" 
        << "Typed characters will be echoed, until ^D is typed.\n"
        << "Note newlines are needed to flush input through UNIX.\n";
    cout.flush();

    do {
        ch = synchConsoleIn->GetChar();
        if(ch != EOF) synchConsoleOut->PutChar(ch);   // echo it!
    } while (ch != EOF);

    cout << "\n";

}

//----------------------------------------------------------------------
// Kernel::NetworkTest
//      Test whether the post office is working. On machines #0 and #1, do:
//
//      1. send a message to the other machine at mail box #0
//      2. wait for the other machine's message to arrive (in our mailbox #0)
//      3. send an acknowledgment for the other machine's message
//      4. wait for an acknowledgement from the other machine to our 
//          original message
//
//  This test works best if each Nachos machine has its own window
//----------------------------------------------------------------------

void
Kernel::NetworkTest() {

    if (hostName == 0 || hostName == 1) {
        // if we're machine 1, send to 0 and vice versa
        int farHost = (hostName == 0 ? 1 : 0); 
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "Hello there!";
        char *ack = "Got it!";
        char buffer[MaxMailSize];

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = farHost;         
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;

        // Send the first message
        postOfficeOut->Send(outPktHdr, outMailHdr, data); 

        // Wait for the first message from the other machine
        postOfficeIn->Receive(0, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box " 
                                                << inMailHdr.from << "\n";
        cout.flush();

        // Send acknowledgement to the other machine (using "reply to" mailbox
        // in the message that just arrived
        outPktHdr.to = inPktHdr.from;
        outMailHdr.to = inMailHdr.from;
        outMailHdr.length = strlen(ack) + 1;
        postOfficeOut->Send(outPktHdr, outMailHdr, ack); 

        // Wait for the ack from the other machine to the first message we sent
	postOfficeIn->Receive(1, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box " 
                                                << inMailHdr.from << "\n";
        cout.flush();
    }

    // Then we're done!
}

//this function will be invoked when there is a page fault exception
//and it will allocate a physical page to the address which cause the page fault
void Kernel::allocatePage(int address){
    
    //calculate virtual page number
    unsigned int VPN = (unsigned) address/PageSize;

    //find a free physical page
    int PPN = memoryManager->FindAndSet();

    //there is no free physical page, need page swapping, the page replacement policy is FIFO
    if (PPN == -1) {
        //find the earilest allocated physical page, and evict it out
        int order = PPT[0].pageOrder;
        PPN = 0;
        for (int i=0; i<NumPhysPages; i++) {
            if(PPT[i].pageOrder < order) {
                order = PPT[i].pageOrder;
                PPN = i;
            }
        }

        Thread* temp = PPT[PPN].threadOnThisPage;

        if(temp != NULL){
            int tempVPN = PPT[PPN].virtualPage;
            //write content back from main memory to swap file if the physical page is dirty
            if(temp->space->pageTable[tempVPN].dirty){
                temp->space->addrSpace->WriteAt(&(machine->mainMemory[PPN*PageSize]), PageSize, tempVPN*PageSize);
                temp->space->pageTable[tempVPN].dirty = FALSE;
            }
            //update page table entry of the virutal page which is mapped to the physical page
            temp->space->pageTable[tempVPN].valid = FALSE;
        }
    }

    //update virutal page entry of current thread
    machine->pageTable[VPN].virtualPage = VPN;
    machine->pageTable[VPN].physicalPage = PPN;
    machine->pageTable[VPN].valid = TRUE;
    machine->pageTable[VPN].dirty = FALSE;
    machine->pageTable[VPN].readOnly = FALSE;
    machine->pageTable[VPN].use = FALSE;

    //update information of physical page entry
    PPT[PPN].virtualPage = VPN;
    PPT[PPN].threadOnThisPage = currentThread;
    PPT[PPN].pageOrder = PAGE_ORDER++;

    //zero out the physical page memory to avoid information leakage
    bzero(&(machine->mainMemory[PPN*PageSize]), PageSize);

    //load content from current thread swap file into main memory
    currentThread->space->addrSpace->ReadAt(&(machine->mainMemory[PPN*PageSize]), PageSize, VPN*PageSize);

    //run the failing instruction again
    machine->Run();
}

