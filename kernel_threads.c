
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
  ptcb->exit_cv = COND_INIT;
  ptcb->refcount = 0;

  rlnode_init(& ptcb->ptcb_list_node ,NULL);
  
}
//test
//test2
//test3
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
	return (Tid_t) cur_thread()->ptcb ; // Return PTCB better
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval) //
{
  // ----- ELEGXOI POU PREPEI NA GINOUN EDO -----
  // 1) An to dosmeno tid den yparxei  apo listapcb arrlist_find
  // 2) An to tid tis parametrou einai to tid tou thread poy kalei thn join
  // 3) An to tid poy dinetai anikei se thread poy exei kanei detach
  // 4) Ta 2 threads (auto poy kalei kai ayto pou kaleitai) na anikoun sto idio PCB

  PTCB* cur_thread_ptcb = (PTCB*) sys_ThreadSelf();
  PTCB* thread_to_join_in = (PTCB*) tid;
  PCB* parent_pcb = cur_thread_ptcb->tcb->owner_pcb;

  // ----- TESTS INIT -----
  if (rlist_find(&(parent_pcb->ptcb_list), &(cur_thread_ptcb->ptcb_list_node), NULL) == NULL)
    return -1;
  if (cur_thread_ptcb == ((PTCB*) tid) || (cur_thread_ptcb -> detached) == 1 || cur_thread_ptcb->tcb->owner_pcb == ((PTCB*) tid)->tcb->owner_pcb)
    return -1; 
  
  // ----- TESTS PASSED -----
  cur_thread_ptcb->refcount++; // refcount++

  while (cur_thread_ptcb->exited == 0 && cur_thread_ptcb->detached == 0)      //while thread not detached and not exited
    kernel_wait(&(((PTCB*) tid) -> exit_cv), SCHED_USER);    // Made current thread get into waiting list of the thread that exists as a parameter
  
  if (thread_to_join_in->detached == 1) //return -1 if t2 was detached
    return -1;
  
  if (exitval != NULL)    // if exit val is not null *exit_val == ptcb->*exit_val
    cur_thread_ptcb->exitval = *exitval;
  
  cur_thread_ptcb->refcount--;    // refcount--
  if (cur_thread_ptcb->refcount == 0)      // if refcount == 0 then remove ptcb from curproc
    rlist_remove(&(cur_thread_ptcb->ptcb_list_node));

	return 0;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{

  // kanw ta testakia poy grafei sto brief toy tinyos.h
  // prosexe gia tin rlist_find
  // ama perasoun ola ta testakia kane broadcast
  
  PTCB* ptcb_of_thread = (PTCB*) tid;
  PCB* parent_pcb = ptcb_of_thread->tcb->owner_pcb;

  if (ptcb_of_thread->exited == 1)   // Check if the given tid corresponds to an exited thread
    return -1;
  
  if (rlist_find(&(parent_pcb->ptcb_list), &(ptcb_of_thread->ptcb_list_node), NULL) == NULL)    // Checking if the given tid actually exists
    return -1;

  ptcb_of_thread->detached = 1;    // Making the tid detached
  kernel_broadcast(&(ptcb_of_thread->exit_cv));    // Waking up all the waiting threads and telling them that they aint gonna join it

	return 0;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval) // Tzoutzouko pare aythn prwta giati ti xreiazomaste sthn join
{

}

