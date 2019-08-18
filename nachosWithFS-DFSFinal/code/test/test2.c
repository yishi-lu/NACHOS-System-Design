#include "syscall.h"

int
main()
{
  
   char *name1 = "File2_1";

   char *name2 = "File2_2";

   char buf[50];

   Create(name1, 10);

   Write(name1, 10, "This is file 1_1 abcdefghijklmnopqrstuvwxyz", 43);


   Create(name2, 10);

   Write(name2, 10, "This is file 2_2 abcdefghijklmnopqrstuvwxyz", 43);



  //Write("test", 4, "abcdefghijklmnopqrstuvwxyz", 26);
  //Read("test", 4, buf, 1000);

 // Print(buf, 1000);
 ReadTo("0/File1", 10, buf, 50);
 Print(buf, 50);
  Listening();
    
  Exit(0);
}