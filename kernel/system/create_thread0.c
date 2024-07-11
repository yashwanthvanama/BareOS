#include <barelib.h>
#include <interrupts.h>
#include <bareio.h>
#include <shell.h>
#include <thread.h>
#include <syscall.h>

void create_thread0(char* args){
    int32 threadid = create_thread((void (*)(void))shell, "\0", 0);
    resume_thread(threadid);
    while(thread_table[threadid].state != TH_DEFUNCT);
    return;
}