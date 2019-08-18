#include "syscall.h"

int
main()
{
 
    char *content="I am node 3，I will change all of you!!!!";
    char buf[50];

   WriteTo("1/File2_1", 10, content, 50);
   ReadTo("1/File2_1",10,buf,50);
   Print(buf, 50);
    
  Exit(0);
}