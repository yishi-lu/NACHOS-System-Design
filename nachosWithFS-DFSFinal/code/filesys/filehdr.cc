// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

//-----------------------DoubleIndirectHeader---------------------------

DoubleIndirectHeader::DoubleIndirectHeader(){
    //initialize indirect header, we set all them to -1 because they are not assigned sector now
    for(int i=0; i<sectorPerIndirect; i++){
        sectorPointer[i] = -1;
    }
}
//fecth doubly indirect header from sector
void DoubleIndirectHeader::FetchFrom(int sector){
    kernel->synchDisk->ReadSector(sector, (char *)this);
}
//store doubly indirect header to sector
void DoubleIndirectHeader::WriteBack(int sector){
    kernel->synchDisk->WriteSector(sector, (char *)this);
}    

//-----------------------IndirectHeader---------------------------------

IndirectHeader::IndirectHeader(){
    //initialize  sector array, we set all them to -1 because they are not assigned sector now
    for(int i=0; i<sectorPerIndirect; i++){
        sectorPointer[i] = -1;
    }
}
//fecth indirect header from sector
void IndirectHeader::FetchFrom(int sector){
    kernel->synchDisk->ReadSector(sector, (char *)this);
}   
//store indirect header to sector
void IndirectHeader::WriteBack(int sector){
    kernel->synchDisk->WriteSector(sector, (char *)this);
}     

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
    
    //space of direct table is enough
    if(numSectors < NumDirect){
        cout<<"Allocated sector for direct table: "<<endl;
        for (int i = 0; i < numSectors; i++) {
    	dataSectors[i] = freeMap->FindAndSet();
        cout<<dataSectors[i]<<", ";
    	// since we checked that there was enough free space,
    	// we expect this to succeed
    	ASSERT(dataSectors[i] >= 0);
        }
        cout<<endl;
    }
    else {
        int allocatedSector = 0;//variable to track how many sector we allocated

        //we first allocate all direct table
        cout<<"Allocated sector for direct table: "<<endl;
        for (int i = 0; i <NumDirect; i++) {
            dataSectors[i] = freeMap->FindAndSet();
            cout<<dataSectors[i]<<", ";
            allocatedSector++;
            // since we checked that there was enough free space,
            // we expect this to succeed
            ASSERT(dataSectors[i] >= 0);
        }
        cout<<endl;
        //direct table is full, we need to use double indirect table to allocate more sector
        if(allocatedSector < numSectors){
            //allocate sector for storing doubly indirect header
            DoubleIndirectHeader *doubleHeader = new DoubleIndirectHeader();
            doublyIndirectSector = freeMap->FindAndSet();
            cout<<"Allocated sector for doubly indirect table: "<<endl<<doublyIndirectSector<<endl;

            //allocate indirect header
            IndirectHeader *indirectHeader;
            for(int i=0; i<sectorPerIndirect && allocatedSector<numSectors; i++){
                indirectHeader = new IndirectHeader();
                //if current indirect header is not used, then we allocate it by assign it a sector number
                doubleHeader->sectorPointer[i] = freeMap->FindAndSet();

                cout<<"Allocated sector for indirect table: "<<endl<<doubleHeader->sectorPointer[i]<<endl;

                cout<<"Allocated sector: "<<endl;
                //allocate sector to indirect table
                for(int i=0; i<sectorPerIndirect && allocatedSector<numSectors; i++){
                    //we allocate it by assign it a sector number
                    indirectHeader->sectorPointer[i] = freeMap->FindAndSet();

                    cout<<indirectHeader->sectorPointer[i]<<", ";
                    allocatedSector++;
                }
                cout<<endl;
                //write the indirect header back to sector
                indirectHeader->WriteBack(doubleHeader->sectorPointer[i]);
                delete indirectHeader;
            }
            cout<<endl;
            //write the doubly indirect header back to sector
            doubleHeader->WriteBack(doublyIndirectSector);
            delete doubleHeader;
        }
    }

    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(PersistentBitmap *freeMap)
{
    if(numSectors < NumDirect){
        for (int i = 0; i < numSectors; i++) {
    	//ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
    	freeMap->Clear((int) dataSectors[i]);
        }
    }
    else {
        //we first deallocate all direct table
        for (int i = 0; i < NumDirect; i++) {
            //ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
        }

        //if doubly indirect header is also in use, we need to deallocate it as well
        //because doubly indirect header is in use, which means indirect header is in use as well
        //we first fetch all indirect header, and deallocate all their sector
        //then we deallocate the doubly indirect header
        DoubleIndirectHeader *doubleHeader = new DoubleIndirectHeader();
        doubleHeader->FetchFrom(doublyIndirectSector);
        IndirectHeader *indirectHeader;
        for(int i=0; i<sectorPerIndirect; i++){
            if(doubleHeader->sectorPointer[i] == -1) continue;
            else {
                indirectHeader = new IndirectHeader();
                indirectHeader->FetchFrom(doubleHeader->sectorPointer[i]);
                for(int i=0; i<sectorPerIndirect; i++){
                    if(indirectHeader->sectorPointer[i] == -1) continue;
                    else {
                        freeMap->Clear(indirectHeader->sectorPointer[i]);
                        indirectHeader->sectorPointer[i] = -1;
                    }
                }
                freeMap->Clear(doubleHeader->sectorPointer[i]);
                doubleHeader->sectorPointer[i] = -1;
                delete indirectHeader;
            }
        }
        freeMap->Clear(doublyIndirectSector);
        doublyIndirectSector = 0;
        delete doubleHeader;
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    kernel->synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{   
    int sectorOffset = offset/SectorSize;
    //if the offset is in the direct table, then we resture it immediately
    if(sectorOffset < NumDirect) return(dataSectors[sectorOffset]); 
    //otherwise, we need to compute its offset
    else{

        //fetch the doubly indirect header from disk
        DoubleIndirectHeader *doubleHeader = new DoubleIndirectHeader();
        doubleHeader->FetchFrom(doublyIndirectSector);

        //fetch the indirect header from disk
        IndirectHeader *indirectHeader = new IndirectHeader();
        indirectHeader->FetchFrom(doubleHeader->sectorPointer[sectorOffset/sectorPerIndirect]);

        //get the sector number
        int result = indirectHeader->sectorPointer[sectorOffset%sectorPerIndirect]; 

        delete indirectHeader;
        delete doubleHeader;
        //ASSERT(result >= 0);
        return result;
    }
}

bool FileHeader::extendFileSize(PersistentBitmap *freeMap, int fileSize){

    int originalSize = FileLength();
    int originalSector = divRoundUp(originalSize, SectorSize);
    int extraSectors = divRoundUp(fileSize, SectorSize);

    //there is no enough free sector
    if (freeMap->NumClear() < extraSectors)
        return FALSE;       // not enough space

    //there is no enough sector
    if(originalSector + extraSectors > NumSectors) 
        return FALSE;

    int allocatedSector = 0;
    //allocate direct table first
    if(originalSector < NumDirect){
        cout<<"Allocated sector for direct table: "<<endl;
        for(int i=0; i<NumDirect && i<extraSectors; i++){
            dataSectors[i] = freeMap->FindAndSet();
            allocatedSector++;
            cout<<dataSectors[i]<<", ";
            ASSERT(dataSectors[i] >= 0);
        }
    }

    cout<<endl;

    //direct table is full, allocate more sector by doubly indirect table
    if(allocatedSector < extraSectors){
        DoubleIndirectHeader *doubleHeader = new DoubleIndirectHeader();

        //double indirect header is not allocated, allocate sector for it
        if(doublyIndirectSector == -1) doublyIndirectSector = freeMap->FindAndSet();
        //if the double indirect header is already in using, fetch it from disk
        else doubleHeader->FetchFrom(doublyIndirectSector);
        cout<<"Allocated/Fetch sector for doubly indirect table: "<<endl<<doublyIndirectSector<<endl;
        //the rest is similar as function Allocation()
        IndirectHeader *indirectHeader;
        for(int i=0; i<sectorPerIndirect && allocatedSector<extraSectors; i++){
            indirectHeader = new IndirectHeader();
            //if we already allocate sector for the indirect header, then we just fetch it from sector
            //otherwise, we allocate a sector for it
            if(doubleHeader->sectorPointer[i] == -1) doubleHeader->sectorPointer[i] = freeMap->FindAndSet();
            else indirectHeader->FetchFrom(doubleHeader->sectorPointer[i]);
            cout<<"Allocated sector for indirect table: "<<endl<<doubleHeader->sectorPointer[i]<<endl;

            cout<<"Allocated sector: "<<endl;
            for(int i=0; i<sectorPerIndirect && allocatedSector<extraSectors; i++){
                //if we already allocate a sector to the element of indirect header, then we move to next element
                //otherwise, we allocate a sector to the element
                if(indirectHeader->sectorPointer[i] == -1) indirectHeader->sectorPointer[i] = freeMap->FindAndSet();
                else continue;
                allocatedSector++;
                cout<<indirectHeader->sectorPointer[i]<<", ";
            }
            cout<<endl;
            indirectHeader->WriteBack(doubleHeader->sectorPointer[i]);
            delete indirectHeader;
        }
        cout<<endl;
        doubleHeader->WriteBack(doublyIndirectSector);
        delete doubleHeader;
    }

    //after extension, we update file size and number of sector of current file
    numBytes = numBytes + fileSize;
    numSectors = numSectors + extraSectors;

    return TRUE;

}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	kernel->synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}
