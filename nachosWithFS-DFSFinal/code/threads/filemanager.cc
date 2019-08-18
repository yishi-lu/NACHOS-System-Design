
#include "synch.h"
#include "filemanager.h"
FileManager::FileManager(){
	for(int i=0;i<FILEMAXNUMBER;i++){
		semaphoreW[i]=new Semaphore("filesavers",1);
		semaphoreR[i]=new Semaphore("filesavers",1);
	}
	currIndex=0;
	numOfReader=0;
}
fileStatus
FileManager::ReadStatus(char *filename){
	return stats[getFileIndex(filename)];
}
void
FileManager::RequestReadAt(char *filename){
	int thisIndex=getFileIndex(filename);
	Semaphore *s=semaphoreW[thisIndex];
	ASSERT(thisIndex!=-1);
	if (numOfReader=0)
	{
		/* code */
		semaphoreW[thisIndex]->P();
	}
	numOfReader++;
}
void
FileManager::RequestWriteAt(char *filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	semaphoreW[thisIndex]->P();
	semaphoreR[thisIndex]->P();
}
void
FileManager::ReleaseReadAt(char *filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	Semaphore *s=semaphoreW[thisIndex];
	numOfReader--;
	if (numOfReader==0);
	{
		/* code */
		s->V();
	}
	
}
void
FileManager::ReleaseWriteAt(char *filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	semaphoreW[thisIndex]->V();
	semaphoreR[thisIndex]->V();
}
void
FileManager::DeleteFile(char *filename){
	for (int i = 0; i < currIndex-1; ++i)
	{
		/* code */
		if (filetables[i]!=NULL)
		{
			/* code */
			if (strcmp(filetables[i],filename)==0)
			{
				/* code */
				filetables[i]==NULL;
				return;
			}
		}
	}
}
void
FileManager::DeleteAt(char*filename){
	int thisIndex=getFileIndex(filename);
	ASSERT(thisIndex!=-1);
	semaphoreW[thisIndex]->P();
	semaphoreR[thisIndex]->P();
	DeleteFile(filename);
	semaphoreR[thisIndex]=new Semaphore("filesavers",1);
	semaphoreW[thisIndex]=new Semaphore("filesavers",1);
}

void
FileManager::AddNewFile(char *filename){
	char *realname=(char *)malloc(1000*sizeof(char));
	strcpy(realname,filename);
	for (int i = 0; i < currIndex-1; ++i)
	{
		if (filetables[i]==NULL)
		{
			filetables[i]=realname;
			return;
		}
	}

	filetables[currIndex]=realname;
	stats[currIndex]=SPACING;
	currIndex++;
	
}
int
FileManager::getFileIndex(char *filename){

	for (int i = 0; i < currIndex+1; ++i)
	{
		/* code */
		if(filetables[i]!=NULL){
			if (strcmp(filetables[i],filename)==0)
			{
			/* code */
			return i;
			}
		}
	}
	return -1;
}
