
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"

void initialize_process_thread_control_block(PTCB* ptcb,TCB* tcb){
  ptcb->tcb = tcb;
  ptcb->task = NULL;
  ptcb->argl=0;
  ptcb->args = NULL;
  ptcb->exitval = 0;
  ptcb->exited = 0;
  ptcb->detached = 0;
  CondVar_init(& ptcb->exit_cv,NULL);
  ptcb->refcount = 0;

  rlnode_init(& ptcb->ptcb_list_node ,NULL);
  
}

/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{
	return NOTHREAD;
}

/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
	return (Tid_t) cur_thread();
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{
	return -1;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
	return -1;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{

}

