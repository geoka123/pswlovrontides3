#ifndef __KERNEL_THREADS_H
#define __KERNEL_THREADS_H


//charge9 was here

#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"


typedef struct process_thread_control_block{
    TCB* tcb;
    Task task;
    int argl;

    void* args;
    int exitval;

    int exited;
    int detached;
    CondVar exit_cv;

    int refcount;
    rlnode ptcb_list_node;
}PTCB;


void initialize_process_thread_control_block();



#endif