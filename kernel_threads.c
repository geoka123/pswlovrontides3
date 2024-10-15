
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_cc.h" // Bazo tin kernel_wait mesa

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
PTCB* sys_ThreadSelf()
{
	return (PTCB*) ((Tid_t) cur_thread()); // Return PTCB better
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval) // Mounakia tin analamvanei o JETT aythn
{
  // ----- ELEGXOI POU PREPEI NA GINOUN EDO -----
  // 1) An to dosmeno tid den yparxei
  // 2) An to tid tis parametrou einai to tid tou thread poy kalei thn join
  // 3) An to tid poy dinetai anikei se thread poy exei kanei detach
  // 4) Ta 2 threads (auto poy kalei kai ayto pou kaleitai) na anikoun sto idio PCB

  
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
void sys_ThreadExit(int exitval) // Tzoutzouko pare aythn prwta giati ti xreiazomaste sthn join
{

}

