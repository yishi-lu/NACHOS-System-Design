#include "copyright.h"
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "synch.h"
#define FILEMAXNUMBER 100
enum fileStatus {READING, WRITING, SPACING};



class FileManager{
public:
	FileManager();
	~FileManager();
	fileStatus stats[FILEMAXNUMBER];
	fileStatus ReadStatus(char *filename);
	void RequestReadAt(char *filename);
	void RequestWriteAt(char *filename);
	void ReleaseReadAt(char *filename);
	void ReleaseWriteAt(char *filename);
	void AddNewFile(char *filename);
	void DeleteAt(char*filename);
private:
	Semaphore *semaphoreW[FILEMAXNUMBER];
	Semaphore *semaphoreR[FILEMAXNUMBER];
	void DeleteFile(char *filename);
	int getFileIndex(char *filename);
	char *filetables[FILEMAXNUMBER];
	int currIndex;
	int numOfReader;

};
#endif