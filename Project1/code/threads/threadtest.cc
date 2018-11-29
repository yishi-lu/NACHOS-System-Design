#include "kernel.h"
#include "main.h"
#include "process.h"

void
SimpleTest(char *which)
{
    
    for(int i=1; i<=20; i++){
        cout<<"current thread: "<<which<<" has run "<<i<<" times. Current Process: ";
        kernel->currentProcess->Print();
    }

    kernel->currentProcess->currentThread->myFinish();

}


void
ThreadTest()
{ 

    Process *a = new Process(1, 5);
    
    Process *b = new Process(2, 5);
    
    Process *c = new Process(3, 3);
    
    Process *d = new Process(4, 2);
    
    Process *e = new Process(5, 1);
    
    a->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "A T1");
    a->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "A T2");
    a->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "A T3");
    a->ForkProcess();

    b->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "B T1");
    b->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "B T2");
    b->ForkProcess();
    
    c->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "C T1");
    c->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "C T2");
    c->ForkProcess();

    d->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "D T1");
    d->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "D T2");
    d->ForkProcess();
    d->ForkProcess();
    d->ForkProcess();

    e->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "E T1");
    e->ForkThread((VoidFunctionPtr) SimpleTest, (void *) "E T2");
    e->ForkProcess();
    e->ForkProcess();

    a->Join();
    d->Join();
    c->Join();
}
