# Nachos System Design project 

### Project1 (Process and Scheduling): 

-Add process as a layer above thread, so kernel schedule process, and process schedule thread.

-Add priority round robin scheduling. user can specify the time quantum by command ./nchos -quantum xxx.
          
### Project2 (System Call and Virtual Memory): 

-Add same system calls, which include fork(), wait(), exit(), write().

-Improve the system to allow multiple user program run at same time.

-Improve the memory allocation system. Now the system run with real virtual memory system. Multiple user program can be run at same time, and multiple user program with larger than main memory size can also be run on the system.

-Add round robin scheduling.

### Project3 (Distributed File System): 

-Improve Nachos basica file system: allowing multiple thread read and write same file at same time. Increase the file size to be as larger as the disk (128KB). Implementing dynamically extend file size.

-DFS: allow communication between different Nachos systems by reading and writing in file, the communication is asychronous.
