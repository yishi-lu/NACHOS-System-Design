#include "syscall.h"

int
main()
{
 
   char buf[50];
   char *name1 = "File1";

   char *name2 = "File2";

 Print("Making 2 files in this nachos!\n",31);
   Create(name1, 10);

   Write(name1, 10, "This is file 1 abcdefghijklmnopqrstuvwxyz", 41);

   Read(name1, 10, buf, 50);

 Print(buf, 50);

   Create(name2, 10);

   Write(name2, 10, "This is file 2 abcdefghijklmnopqrstuvwxyz", 41);

   Print("This nachos starts Listening for requests",43);
   Listening();
  //Write("test", 4, "abcdefghijklmnopqrstuvwxyz", 26);
  //Read("test", 4, buf, 1000);

 // Print(buf, 1000);
 //ReadTo("0/wwww11",buf);
 //Print(buf, 1000);
  
  Exit(0);
}