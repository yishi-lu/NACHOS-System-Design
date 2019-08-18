#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h> 
#include <unistd.h>

int ParseAddress(char *address,char *filename);
int WriteTo(char *addressString, char *content);
int ReadFrom(char *addressString, char *content);
char TrasferString(char *name, char *filename,char *content);
