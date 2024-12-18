// Tzoutzoukos edooo
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_cc.h" // Bazo tin kernel_wait mesa
#include "kernel_streams.h" //gia to warning

void initialize_process_thread_control_block(PTCB* ptcb){
  ptcb->tcb = NULL;
  ptcb->task = NULL;
  ptcb->argl=0;
  ptcb->args = NULL;
  ptcb->exitval = 0;
  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->exit_cv = COND_INIT;
  ptcb->refcount = 0;

  rlnode_init(& ptcb->ptcb_list_node ,ptcb);
  
}

PTCB* acquire_PTCB(){
  PTCB *ptcb = NULL;

  ptcb = xmalloc(sizeof(PTCB));

  return ptcb;
}

/*
  Must be called with kernel_mutex held
*/
void release_PTCB(PTCB* ptcb)
{
  free(ptcb);
}
/** 
 * 
 *
  @brief Create a new thread in the current process.
  */

void start_thread(){
  int exitval;

  TCB* thisthread;
  thisthread = cur_thread();

  Task call =  thisthread->ptcb->task;

  int argl = thisthread->ptcb->argl;
  void* args = thisthread->ptcb->args;

  exitval = call(argl,args);
  ThreadExit(exitval);
} 

Tid_t sys_CreateThread(Task task, int argl, void* args)
{   
  PTCB*  newptcb;
 // newptcb = acquire_PTCB();
  //initialize_process_thread_control_block(newptcb);
  //if(task != NULL){
  newptcb = xmalloc(sizeof(PTCB));
  newptcb->tcb = spawn_thread(CURPROC, start_thread);


  newptcb->tcb->ptcb = newptcb;
  


  newptcb->task = task;
  newptcb->argl = argl;
  newptcb->args = args;

  newptcb->exited = 0;
  newptcb->detached = 0;
  newptcb->exit_cv = COND_INIT;
  newptcb->refcount = 0;
  //if(task != NULL) {

  rlnode_init(& newptcb->ptcb_list_node ,newptcb);
    

  rlist_push_back(& CURPROC->ptcb_list,& newptcb->ptcb_list_node);
  
  CURPROC->thread_count++;//increasing thread count

  wakeup(newptcb->tcb);
  return (Tid_t)newptcb;
  //}
  //return NOTHREAD;  
	
}

/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
	return (Tid_t) (cur_thread()->ptcb); // Return PTCB better
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval) {
    // ----- TESTS INIT -----
    PTCB* thread_to_join_in = (PTCB*) tid;
    PTCB* cur_thread_ptcb = (PTCB*) sys_ThreadSelf();

    if (rlist_find(&(CURPROC->ptcb_list), thread_to_join_in, NULL) == NULL)
      return -1;
    if (cur_thread_ptcb == thread_to_join_in || thread_to_join_in->detached == 1)
      return -1;
    if (tid == NOTHREAD)
      return -1;

    // ----- TESTS PASSED -----
    thread_to_join_in->refcount++;  // refcount++

    while (thread_to_join_in->exited == 0 && thread_to_join_in->detached == 0)
        kernel_wait(&(thread_to_join_in->exit_cv), SCHED_USER);

    if (thread_to_join_in->detached == 1) {
        thread_to_join_in->refcount--;  // Adjust refcount if detached
        return -1;
    }

    if (exitval != NULL) {
        *exitval = thread_to_join_in->exitval;  // Return thread exit value
    }

    thread_to_join_in->refcount--;  // refcount--
    if (thread_to_join_in->refcount == 0) {
        rlist_remove(&thread_to_join_in->ptcb_list_node);  // Clean up if no refs
        free(thread_to_join_in);
    }
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

  if ( rlist_find(& CURPROC->ptcb_list, ptcb_of_thread, NULL) == NULL)
    return -1;

  if (ptcb_of_thread->exited == 1)   // Check if the given tid corresponds to an exited thread
    return -1;
  
  //   if (ptcb_of_thread == cur_thread()->ptcb){
  //     ptcb_of_thread->detached = 1;    // Making the tid detached
  //     kernel_broadcast(&(ptcb_of_thread->exit_cv));
  //   return 0;
  // }
  
  // if (tid == NOTHREAD)
  //   return -1;
  
  // if (CURPROC->thread_count == 1)
  //   return -1;


   ptcb_of_thread->detached = 1;    // Making the tid detached
   kernel_broadcast(&(ptcb_of_thread->exit_cv));    // Waking up all the waiting threads and telling them that they aint gonna join it

	 return 0;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval){
  PCB *curproc = CURPROC; 
  PTCB * ptcb;

  ptcb = cur_thread()->ptcb;
  
  ptcb->exitval = exitval;
  ptcb->exited = 1;
  curproc->thread_count--;
  kernel_broadcast(&(ptcb->exit_cv));

  if(curproc->thread_count==0){
    if(get_pid(curproc)!=1){
      PCB* initpcb = get_pcb(1);
      while(!is_rlist_empty(& curproc->children_list)) {
        rlnode* child = rlist_pop_front(& curproc->children_list);
        child->pcb->parent = initpcb;
        rlist_push_front(& initpcb->children_list, child);
      }

      /* Add exited children to the initial task's exited list 
        and signal the initial task */
      if(!is_rlist_empty(& curproc->exited_list)) {
        rlist_append(& initpcb->exited_list, &curproc->exited_list);
        kernel_broadcast(& initpcb->child_exit);
      }

      /* Put me into my parent's exited list */
      rlist_push_front(& curproc->parent->exited_list, &curproc->exited_node);
      kernel_broadcast(& curproc->parent->child_exit);

    
    }
      assert(is_rlist_empty(& curproc->children_list));
      assert(is_rlist_empty(& curproc->exited_list));
    

  /* 
    Do all the other cleanup we want here, close files etc. 
   */

  /* Release the args data */
    if(curproc->args) {
      free(curproc->args);
      curproc->args = NULL;
    }

    /* Clean up FIDT */
    for(int i=0;i<MAX_FILEID;i++) {
      if(curproc->FIDT[i] != NULL) {
        FCB_decref(curproc->FIDT[i]);
        curproc->FIDT[i] = NULL;
      }
    }

    // CLEAN UP REMAINING PTCBS
    rlnode* ptcb_list_in_proc = &CURPROC->ptcb_list;
    //for (int i=0; i<=rlist_len(ptcb_list_in_proc); i++) {
    rlnode* temp_node;
      //if (is_rlist_empty(ptcb_list_in_proc) == 1)
      //   break;
    while(!is_rlist_empty(ptcb_list_in_proc)){
      temp_node = rlist_pop_front(ptcb_list_in_proc);
      free(temp_node->ptcb);
    }
      //rlist_remove(&temp_node->ptcb->ptcb_list_node);
      //temp_node = temp_node->next;
    //}

    /* Disconnect my main_thread */
    curproc->main_thread = NULL;

    /* Now, mark the process as exited. */
    curproc->pstate = ZOMBIE;
    //osa threads den exoyn ginei join apo alla oste na sbhstei to ptcb toys mesa sthn join tha prepei na sbh
  
  }
  kernel_sleep(EXITED, SCHED_USER);
}

