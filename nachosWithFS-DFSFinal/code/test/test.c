/* add.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int
main()
{

   char buf[30];
   char *name = "testFile";

   Create(name, 10);

   WriteTo(name, 10, "abcdefghijklmnopqrstuvwxyzzzzz", 32);

  //Write("test", 4, "abcdefghijklmnopqrstuvwxyz", 26);
  ReadTo(name, 8, buf, 30);

  Print(buf, 30);

  //Listening();
	//ReadTo("0/wwww11",buf);
	//Print(buf, 1000);
  
  Exit(0);
}


