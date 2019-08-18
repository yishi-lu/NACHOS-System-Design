#include "kernel.h"
#include "main.h"
#include "thread.h"
#include "filemanager.h"

void test1(int which){
    char* input = new char[120000];
    for(int i=0; i<120000; i++){
        if(i<20000) input[i] = 'a';
        else if(i<40000) input[i] = 'b';
        else if(i<60000) input[i] = 'c';
        else if(i<80000) input[i] = 'd';
        else  input[i] = 'e';
    }
    input[119999] = '!';

    cout<<"Test for maximum file size:"<<endl;
    char* t="test";

    kernel->fileSystem->Create(t,124000);
    OpenFile *test=kernel->fileSystem->Open((char *)t);
    test->WriteAt(input, 120000, 0);
    char* temp = new char[120000];
    test->ReadAt(temp, 120000, 0);
    printf("%s\n", temp);
    kernel->fileSystem->Remove((char *)t);
    cout<<endl<<endl<<endl;
}

void test2(int which){
    char* input = new char[120000];
    for(int i=0; i<120000; i++){
        if(i<20000) input[i] = 'a';
        else if(i<40000) input[i] = 'b';
        else if(i<60000) input[i] = 'c';
        else if(i<80000) input[i] = 'd';
        else  input[i] = 'e';
    }
    input[119999] = '!';

    cout<<endl<<"Test for extensible file size:"<<endl;
    char* t="test";
    cout<<"Create file with size 0"<<endl;
    kernel->fileSystem->Create(t,0);
    OpenFile *test=kernel->fileSystem->Open((char *)t);
    cout<<"Write to file with bytes 120000"<<endl;
    test->WriteAt(input, 120000, 0);
    char* temp = new char[120000];
    test->ReadAt(temp, 120000, 0);
    printf("%s\n", temp);
    kernel->fileSystem->Remove((char *)t);
    cout<<endl<<endl<<endl;
}

void test3(int which){
    char* input = new char[1500];
    for(int i=0; i<1500; i++){
        input[i] = 'a';
    }

    cout<<"Test for more than ten files:"<<endl;
    char* u1="test1";
    char* u2="test2";
    char* u3="test3";
    char* u4="test4";
    char* u5="test5";
    char* u6="test6";
    char* u7="test7";
    char* u8="test8";
    char* u9="test9";
    char* u10="test10";
    char* u11="test11";
    char* u12="test12";

    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u1,1000);
    OpenFile *of1=kernel->fileSystem->Open((char *)u1);
    of1->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u2,1000);
    OpenFile *of2=kernel->fileSystem->Open((char *)u2);
    of2->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u3,1000);
    OpenFile *of3=kernel->fileSystem->Open((char *)u3);
    of3->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u4,1000);
    OpenFile *of4=kernel->fileSystem->Open((char *)u4);
    of4->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u5,1000);
    OpenFile *of5=kernel->fileSystem->Open((char *)u5);
    of5->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u6,1000);
    OpenFile *of6=kernel->fileSystem->Open((char *)u6);
    of6->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u7,1000);
    OpenFile *of7=kernel->fileSystem->Open((char *)u7);
    of7->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u8,1000);
    OpenFile *of8=kernel->fileSystem->Open((char *)u8);
    of8->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u9,1000);
    OpenFile *of9=kernel->fileSystem->Open((char *)u9);
    of9->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u10,1000);
    OpenFile *of10=kernel->fileSystem->Open((char *)u10);
    of10->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u11,1000);
    OpenFile *of11=kernel->fileSystem->Open((char *)u11);
    of11->WriteAt(input, 1000, 0);
    cout<<"Create file"<<endl;
    kernel->fileSystem->Create(u12,1000);
    OpenFile *of12=kernel->fileSystem->Open((char *)u12);
    of12->WriteAt(input, 1000, 0);
    
    kernel->fileSystem->Remove((char *)u1);
    kernel->fileSystem->Remove((char *)u2);
    kernel->fileSystem->Remove((char *)u3);
    kernel->fileSystem->Remove((char *)u4);
    kernel->fileSystem->Remove((char *)u5);
    kernel->fileSystem->Remove((char *)u6);
    kernel->fileSystem->Remove((char *)u7);
    kernel->fileSystem->Remove((char *)u8);
    kernel->fileSystem->Remove((char *)u9);
    kernel->fileSystem->Remove((char *)u10);
    kernel->fileSystem->Remove((char *)u11);
    kernel->fileSystem->Remove((char *)u12);
}

void test4(int which){
    char* input = new char[20000];
    for(int i=0; i<20000; i++){
        input[i] = 'a';
        
    }
    input[19999] = '!';

    cout<<"Test for extensible file size 2:"<<endl;
    char* t="test";
    cout<<"Create file with size 0"<<endl;
    kernel->fileSystem->Create(t,0);
    OpenFile *test=kernel->fileSystem->Open((char *)t);
    cout<<"Write to file with bytes 1000"<<endl;
    test->WriteAt(input, 1000, 0);
    cout<<"Write to file with bytes 10000"<<endl;
    test->WriteAt(input, 10000, 1000);
    cout<<"Write to file with bytes 9000"<<endl;
    test->WriteAt(input, 9000, 11000);
    char* temp = new char[20000];
    test->ReadAt(temp, 20000, 0);
    printf("%s\n", temp);
    kernel->fileSystem->Remove((char *)t);
    cout<<endl<<endl<<endl;
}

void
FileTest(int which){
    char* t="wwww";

    OpenFile *of=kernel->fileSystem->Open((char *)t);
    char *input;
    if (which==1)
    {
        input="Bye!";
    }
    else{
        input="Hel!";
    //input[1]=(char)(which+48);
    }
    kernel->filemanager->RequestWriteAt(t);
    for (int i = 0; i < 10; ++i)
    {
        printf("Written once for process %d\n", which);

         of->WriteAt(input, 4, i*4);

         kernel->currentThread->Yield();
    }
    kernel->filemanager->ReleaseWriteAt(t);
    char* temp = new char[200];

        of->ReadAt(temp, 100, 0);
        printf("%d: \n%s\n\n",which, temp);
        //kernel->currentThread->Yield();
}

void
unsafe_FileTest(int which){
   
    char* t="wwww";

    OpenFile *of=kernel->fileSystem->Open((char *)t);
    char *input;
    if (which==1)
    {
        input="Bye!";
    }
    else{
        input="Hel!";
    //input[1]=(char)(which+48);
    }
    //kernel->filemanager->RequestWriteAt(t);
    for (int i = 0; i < 10; ++i)
    {
        printf("Written once for process %d\n", which);

         of->WriteAt(input, 4, i*4);

         kernel->currentThread->Yield();
    }
    //kernel->filemanager->ReleaseWriteAt(t);
    char* temp = new char[200];

        of->ReadAt(temp, 100, 0);
        printf("%d: \n%s\n\n",which, temp);
        //kernel->currentThread->Yield();

}

void DeleteIt(char *filename){
    printf("Deleting want to execute\n" );
  OpenFile *of=kernel->fileSystem->Open(filename);

  kernel->filemanager->DeleteAt(filename);
  bool k=kernel->fileSystem->Remove(filename);
  if (k)
  {
      /* code */
    printf("The file %s is deleted successfully!\n", filename);
  }
  else{
    printf("The file %s is not deleted successfully!\n", filename);
  }
}
void unsafe_DeleteIt(char *filename){
    printf("Deleting want to execute\n" );
  OpenFile *of=kernel->fileSystem->Open(filename);

  //kernel->filemanager->DeleteAt(filename);
  bool k=kernel->fileSystem->Remove(filename);
  if (k)
  {
      /* code */
    printf("The file %s is deleted successfully!\n", filename);
  }
  else{
    printf("The file %s is not deleted successfully!\n", filename);
  }
}

void TestForMutex(){
    
    kernel->fileSystem->Create("wwww",500);
    printf("Doing SAFE file Write/Write Testing...\n Two threads are writing to the same file \"wwww\" with different content at the same time. There are Yields during write operations but there is no Yield between Write and Read.\n");
    Thread *t = new Thread("forked thread");
    t->Fork((VoidFunctionPtr) FileTest, (void *) 1);

    FileTest(0);
    kernel->currentThread->Yield();
}

void unsafe_TestForMutex(){
    kernel->fileSystem->Create("wwww",500);
     printf("Doing UNSAFE file Write/Write Testing...\n Two threads are writing to the same file \"wwww\" with different content at the same time. There are Yields during write operations but there is no Yield between Write and Read.\n");
    Thread *t = new Thread("forked thread");
    t->Fork((VoidFunctionPtr) unsafe_FileTest, (void *) 1);

    unsafe_FileTest(0);
    kernel->currentThread->Yield();
}
void TestForDelete(){
     kernel->fileSystem->Create("wwww",500);
     printf("Tesing for SAFE delete. The output will show where the delete request is genereated and when the delete operation is executed.\n");
    //Thread *t = new Thread("forked thread");
    //t->Fork((VoidFunctionPtr) FileTest, (void *) 1);
    Thread *d=new Thread("Delete file");
    d->Fork((VoidFunctionPtr) DeleteIt, (void *) "wwww");
    FileTest(0);

}
void unsafe_TestForDelete(){
 kernel->fileSystem->Create("wwww",500);
     printf("Tesing for UNSAFE delete. The output will show where the delete request is genereated and when the delete operation is executed.\n");
    //Thread *t = new Thread("forked thread");
    //t->Fork((VoidFunctionPtr) FileTest, (void *) 1);
    Thread *d=new Thread("Delete file");
    d->Fork((VoidFunctionPtr) unsafe_DeleteIt, (void *) "wwww");
    FileTest(0);

}


void
ThreadTest(int i)
{
    Thread *t = new Thread("forked thread");
    if(i == 0) {
        t->Fork((VoidFunctionPtr) test1, (void *) 1);
    }
    if(i == 1) {
        t->Fork((VoidFunctionPtr) test2, (void *) 1);
    }
    if(i == 2) {
        t->Fork((VoidFunctionPtr) test3, (void *) 1);
    }
    if(i == 3) {
        TestForMutex();
    }
    if(i == 4) {
        unsafe_TestForMutex();
    }
    if(i == 5) {
        TestForDelete();
    }
    if(i == 6) {
        unsafe_TestForDelete();
    }
    if(i == 7) {
        t->Fork((VoidFunctionPtr) test4, (void *) 1);
    }
}
