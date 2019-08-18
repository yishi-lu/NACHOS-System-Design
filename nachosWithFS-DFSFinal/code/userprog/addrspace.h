// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize		1024 	// increase this as necessary!

class addrOpenFile {
  public:
    addrOpenFile(int f) { file = f; currentOffset = 0; }    // open the file
    ~addrOpenFile() { Close(file); }            // close the file

    int ReadAt(char *into, int numBytes, int position) { 
            Lseek(file, position, 0); 
        return ReadPartial(file, into, numBytes); 
        }   
    int WriteAt(char *from, int numBytes, int position) { 
            Lseek(file, position, 0); 
        WriteFile(file, from, numBytes); 
        return numBytes;
        }   
    int Read(char *into, int numBytes) {
        int numRead = ReadAt(into, numBytes, currentOffset); 
        currentOffset += numRead;
        return numRead;
            }
    int Write(char *from, int numBytes) {
        int numWritten = WriteAt(from, numBytes, currentOffset); 
        currentOffset += numWritten;
        return numWritten;
        }

    int Length() { Lseek(file, 0, 2); return Tell(file); }
    
  private:
    int file;
    int currentOffset;
};

class addrFileSystem {
  public:
    addrFileSystem() {}

    bool Create(char *name) {
    int fileDescriptor = OpenForWrite(name);

    if (fileDescriptor == -1) return FALSE;
    Close(fileDescriptor); 
    return TRUE; 
    }

    addrOpenFile* Open(char *name) {
      int fileDescriptor = OpenForReadWrite(name, FALSE);

      if (fileDescriptor == -1) return NULL;
      return new addrOpenFile(fileDescriptor);
      }

    bool Remove(char *name) { return Unlink(name) == 0; }

};

class AddrSpace {
  public:
    AddrSpace();			// Create an address space.
    ~AddrSpace();			// De-allocate an address space

    bool Load(char *fileName);		// Load a program into addr space from
                                        // a file
					// return false if not found

    void Execute();             	// Run a program
					// assumes the program has already
                                        // been loaded

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 

    // Translate virtual address _vaddr_
    // to physical address _paddr_. _mode_
    // is 0 for Read, 1 for Write.
    ExceptionType Translate(unsigned int vaddr, unsigned int *paddr, int mode);

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

};

#endif // ADDRSPACE_H
