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
#include "sender.h"
#include "filemanager.h"
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

#define PORT 9094

void Listening(int node){  
    int valread, new_socket, server_fd; 
    struct sockaddr_in address_in; 
    int option = 1; 
    int addlength = sizeof(address_in); 
    
    char *hello = "Hello from server"; 
       

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        printf("socket create fails"); 
        exit(EXIT_FAILURE); 
    } 
       

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &option, sizeof(option))) 
    { 
        printf("setsockopt function fails"); 
        exit(EXIT_FAILURE); 
    } 
    address_in.sin_family = AF_INET; 
    address_in.sin_addr.s_addr = INADDR_ANY;
    printf("Listening from %d\n",(PORT+node) );
    address_in.sin_port = htons( PORT+node); 
       
    if (bind(server_fd, (struct sockaddr *)&address_in,  
                                 sizeof(address_in))<0) 
    { 
        printf("bind function fails"); 
        exit(EXIT_FAILURE); 
    } 

    if (listen(server_fd, 3) < 0) 
    { 
        printf("listen function fails"); 
        exit(EXIT_FAILURE); 
    } 

    while(TRUE){
    if ((new_socket = 
    	accept(server_fd, (struct sockaddr *)&address_in,  
                       (socklen_t*)&addlength))<0) 
    { 
        printf("accept function fails"); 
        exit(EXIT_FAILURE); 
    } 
    
    char request[1024] ={0}; 
    char type='m';
    char *filename=new char[20];
    char *filecontent=new char[1000];
    valread = read( new_socket , request, 1024); 
    type=TrasferString(request,filename,filecontent);
    printf("Request type: %c\n",type);
    printf("Request type: %s filename Length:%d \n",filename ,strlen(filename));
    OpenFile *of=kernel->fileSystem->Open(filename);
    if(type=='1'){   
        printf("%s\n", filecontent);
	    
	kernel->filemanager->RequestWriteAt(filename);
        of->WriteAt(filecontent, 1000, 0);
	kernel->filemanager->ReleaseWriteAt(filename);   
	    
        char *sc_message="Write successfully!";
        send(new_socket , sc_message , strlen(sc_message) , 0 ); 
    }
    else{
    char* temp = new char[1024];

    kernel->filemanager->RequestReadAt(filename);    
    of->ReadAt(temp, 1024, 0);
    kernel->filemanager->ReleaseReadAt(filename);
	    
    send(new_socket , temp , strlen(temp) , 0 ); 
    printf("File %s sent\n",filename);
    }
    
}

}


void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

	SysHalt();

	ASSERTNOTREACHED();
	break;

      case SC_Add:
	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
	
	/* Process SysAdd Systemcall*/
	int result;
	result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));

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
	case SC_Create:{

		int addr = (int)kernel->machine->ReadRegister(4);
	  	int size = (int)kernel->machine->ReadRegister(5);

	  	int readSize = 0;
	  	char name[size+1];
	  	name[size] = '\0';

	  	while(readSize < size){

	  		kernel->machine->ReadMem(addr++, 1, (int* )&name[readSize++]); 

	  	}

	  	kernel->fileSystem->Create((char *)name, 1000);
	  	//OpenFile *of=kernel->fileSystem->Open((char *)name);

	  	//of->WriteAt(content, contentSize, 0);

		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

	  return;
	
		ASSERTNOTREACHED();

		break;
	}
	case SC_Print:{

	  	int address = (int)kernel->machine->ReadRegister(4);
	  	int size = (int)kernel->machine->ReadRegister(5);

	  	int readSize = 0;
	  	char content[size+1];
	  	content[size] = '\0';

	  	while(readSize < size){
	  		kernel->machine->ReadMem(address++, 1, (int* )&content[readSize]); 

		  	
		  	cout<<content[readSize++];

	  	}

	  	cout<<endl;

	  	kernel->machine->WriteRegister(2, (int)size);

	  	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
		  
		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+  4);

	  	return;
	  	ASSERTNOTREACHED();
	  	break;
	}
	case SC_WriteTo:{

		int nameAddr = (int)kernel->machine->ReadRegister(4);
	  	int nameSize = (int)kernel->machine->ReadRegister(5);
	  	int contentAddr = (int)kernel->machine->ReadRegister(6);
	  	int contentSize = (int)kernel->machine->ReadRegister(7);

	  	int readSize = 0;
	  	char name[nameSize+1];
	  	name[nameSize] = '\0';

	  	while(readSize < nameSize){
	  		kernel->machine->ReadMem(nameAddr++, 1, (int* )&name[readSize++]); 
	  	}

	  	readSize = 0;
	  	char content[contentSize+1];
	  	content[contentSize] = '\0';

	  	while(readSize < contentSize){
	  		
	  		kernel->machine->ReadMem(contentAddr++, 1, (int* )&content[readSize++]); 
	  	}

	  	OpenFile *of;

	  	if(name[1] != '/'){
	  		of = kernel->fileSystem->Open((char *)name);
	  		//cout<<name[1]<<endl;
			kernel->filemanager->RequestWriteAt(name);
	  		of->WriteAt(content, contentSize, 0);
			kernel->filemanager->ReleaseReadAt(name);
	  	}
	  	else WriteTo(name, content);

	  	kernel->machine->WriteRegister(2, (int)contentSize);
		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

	  return;
	
		ASSERTNOTREACHED();

		break;
	}
	case SC_ReadTo:{

		int nameAddr = (int)kernel->machine->ReadRegister(4);
	  	int nameSize = (int)kernel->machine->ReadRegister(5);
	  	int contentAddr = (int)kernel->machine->ReadRegister(6);
	  	int contentSize = (int)kernel->machine->ReadRegister(7);

	  	int readSize = 0;
	  	char name[nameSize+1];
	  	name[nameSize] = '\0';

	  	while(readSize < nameSize){
	  		kernel->machine->ReadMem(nameAddr++, 1, (int* )&name[readSize++]); 
	  	}

	  	readSize = 0;
	  	char content[contentSize+1];
	  	content[contentSize] = '\0';

	  	if(name[1] != '/'){
	  		OpenFile *of=kernel->fileSystem->Open((char *)name);
	  		char* temp = new char[contentSize];
			
		kernel->filemanager->RequestReadAt(name);	
    		of->ReadAt(temp, contentSize, 0);
		kernel->filemanager->ReleaseReadAt(name);
	  		readSize = 0;

		  	while(readSize < contentSize){
		  		
		  		kernel->machine->WriteMem(contentAddr++, 1, temp[readSize++]); 
		  	}
	  	}
	  	else ReadFrom(name, content);

		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

	  return;
	
		ASSERTNOTREACHED();

		break;
	}
	case SC_Read:{
		int nameAddr = (int)kernel->machine->ReadRegister(4);
	  	int nameSize = (int)kernel->machine->ReadRegister(5);
	  	int contentAddr = (int)kernel->machine->ReadRegister(6);
	  	int contentSize = (int)kernel->machine->ReadRegister(7);

	  	int readSize = 0;
	  	char name[nameSize+1];
	  	name[nameSize] = '\0';

	  	while(readSize < nameSize){
	  		kernel->machine->ReadMem(nameAddr++, 1, (int* )&name[readSize++]); 
	  	}

	  	//kernel->fileSystem->Create((char *)name, 1000);

	  	OpenFile *of=kernel->fileSystem->Open((char *)name);

	  	//of->WriteAt("content content content", 30, 0);

	  	char* temp = new char[contentSize];
		kernel->filemanager->RequestReadAt(name);
    	of->ReadAt(temp, contentSize, 0);
		kernel->filemanager->ReleaseReadAt(name);

    	//printf("%s\n", temp);


	  	readSize = 0;

	  	while(readSize < contentSize){
	  		
	  		kernel->machine->WriteMem(contentAddr++, 1, temp[readSize++]); 
	  	}

	  	

	  	kernel->machine->WriteRegister(2, (int)contentSize);
		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	    /* set next programm counter for brach execution */
	    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

	    return;
	
		ASSERTNOTREACHED();

		break;
	}
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
	case SC_Write:{

	  	int nameAddr = (int)kernel->machine->ReadRegister(4);
	  	int nameSize = (int)kernel->machine->ReadRegister(5);
	  	int contentAddr = (int)kernel->machine->ReadRegister(6);
	  	int contentSize = (int)kernel->machine->ReadRegister(7);

	  	int readSize = 0;
	  	char name[nameSize+1];
	  	name[nameSize] = '\0';

	  	while(readSize < nameSize){
	  		kernel->machine->ReadMem(nameAddr++, 1, (int* )&name[readSize++]); 
	  	}

	  	readSize = 0;
	  	char content[contentSize+1];
	  	content[contentSize] = '\0';

	  	while(readSize < contentSize){
	  		
	  		kernel->machine->ReadMem(contentAddr++, 1, (int* )&content[readSize++]); 
	  	}

	  	//kernel->fileSystem->Create((char *)name, 1000);

	  	OpenFile *of=kernel->fileSystem->Open((char *)name);

		kernel->filemanager->RequestReadAt(name);
	  	of->WriteAt(content, contentSize, 0);
		kernel->filemanager->ReleaseReadAt(name);

	  	kernel->machine->WriteRegister(2, (int)contentSize);
		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	    /* set next programm counter for brach execution */
	    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

	  return;
	
		ASSERTNOTREACHED();

		break;
	}
	case SC_Listening:
	{	

		Listening(kernel->hostName);

		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	    /* set next programm counter for brach execution */
	    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

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
