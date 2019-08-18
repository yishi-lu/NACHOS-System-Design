// filehdr.h 
//  Data structures for managing a disk file header.  
//
//  A file header describes where on disk to find the data in a file,
//  along with other information about the file (for instance, its
//  length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "pbitmap.h"

#define sectorPerIndirect 32 //each indirect header can have 32 sector
#define sectorPerDoublyIndirect 32 //doubly indirect header have 32 indirect header
#define NumDoublyIndirect 1 //we have one doubly indirect header
#define NumDirect   29 //we have 29 direct header
//the maximum file size now is (128*29 + 32*32*128) > 128KB
#define MaxFileSize     (NumDirect*SectorSize + sectorPerIndirect*sectorPerDoublyIndirect*SectorSize)

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

//class for doubly indirect header, it points to 32 indirect header
class DoubleIndirectHeader{
    public:
        //initializes doubly indirect header object 
        DoubleIndirectHeader();

        void FetchFrom(int sectorNumber);   

        void WriteBack(int sectorNumber);   

        //array for storing sector number for indirect header, so we can fetch them
        int sectorPointer[NumSectors];
};

//class for indirect header, it points to 32 sector 
class IndirectHeader{
    public:
        //initializes indirect header object 
        IndirectHeader();

        void FetchFrom(int sectorNumber);   

        void WriteBack(int sectorNumber);   

        //array for storing sector number, so we can fetch them
        int sectorPointer[NumSectors];
};

class FileHeader {
  public:
    bool Allocate(PersistentBitmap *bitMap, int fileSize);// Initialize a file header, 
                        //  including allocating space 
                        //  on disk for the file data
    void Deallocate(PersistentBitmap *bitMap);  // De-allocate this file's 
                        //  data blocks

    void FetchFrom(int sectorNumber);   // Initialize file header from disk
    void WriteBack(int sectorNumber);   // Write modifications to file header
                    //  back to disk

    int ByteToSector(int offset);   // Convert a byte offset into the file
                    // to the disk sector containing
                    // the byte

    int FileLength();           // Return the length of the file 
                    // in bytes

    void Print();           // Print the contents of the file.

    //function to allocate extended space for current file
    bool extendFileSize(PersistentBitmap *bitMap, int fileSize);

  private:
    int numBytes;           // Number of bytes in the file
    int numSectors;         // Number of data sectors in the file
    int dataSectors[NumDirect];     // Disk sector numbers for each data 
                    // block in the file
    //store the sector number which store the double indirect header
    int doublyIndirectSector = -1;
};

#endif // FILEHDR_H
