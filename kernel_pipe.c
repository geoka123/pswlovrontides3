#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_pipe.h"
#include "kernel_proc.h"
#include "kernel_dev.h"
#include "kernel_cc.h" // Bazo tin kernel_wait mesa

 //gia to warning
#include "kernel_sched.h"

// Vale parametrous twn read kai write
int invalidFunction_read(){
	return -1;
}

int invalidFunction_write(){
	return -1;
}
	
void* invalidFunction2(){
	return NULL;
}


int pipe_write(void* ppcb , const char *buf , unsigned int n){// den exo katalabei ayto to N ti rolo paizei . An prokeitai gia to size toy buffer tote mallon tha prepei na ginei define edw kai oxi sthn kernel_pipe.h
	// ----- ELEGXOI -----
	// Elegxo an ppcb einai null
	// Writer h reader einai closed -> return -1
	// Buffer einai gemato -> kano kernel_wait sti has_space sto while 1) Oso o reader != null kai 2) Oso buffer einai full
	// An bgo apo while kai reader == null -> grafo kai return 0
	// An oxi apo to shmeio pou exo meinei grafo ena ena ta chars mexri na teleiosei
	
	//kano return posa byte egrapsa
	PPCB* my_ppcb = (PPCB*)ppcb;

	unsigned int totalChars = n;

	if(my_ppcb == NULL){
		return -1;
	}

	if(my_ppcb->reader==NULL || my_ppcb->writer == NULL){
		return -1;
	}

	while(my_ppcb->reader != NULL && my_ppcb->writer != NULL && my_ppcb-> free_space == 0){ // edo ligo thema
		kernel_wait(&(my_ppcb->has_space),SCHED_PIPE); 
	}

	if(my_ppcb->reader == NULL || my_ppcb->writer == NULL){
		return -1;
	}

	int charsWritten = 0;// slash 0
	int buffer_index = 0;

	while(my_ppcb->free_space > 0){
		if(totalChars>buffer_index+1){
			my_ppcb->buffer[my_ppcb->w_position] = buf[buffer_index];
			charsWritten++;
			buffer_index++;
			my_ppcb->w_position = (my_ppcb->w_position + 1 )% (PIPE_BUFFER_SIZE-1);
			my_ppcb->free_space--;
		}
		else{
			my_ppcb->buffer[my_ppcb->w_position] = '\0';
			charsWritten++;
			my_ppcb->w_position = (my_ppcb->w_position + 1 )% (PIPE_BUFFER_SIZE-1);
			my_ppcb->free_space--;
			break;
		}
	}
	
	kernel_broadcast(&(my_ppcb->has_data));
	
	return charsWritten;
}

int pipe_read(void* ppcb ,char *buf , unsigned int n){
	// ----- ELEGXOI -----
	// An reader
	// Elegxo an ppcb einai null
	// An einai adeios o buffer && ppcb != null-> kernel_wait sto cv toy has_data
	// Elegxo an writer einai null -> diavase
	// Elegxo an o buffer exei dedomena
	PPCB* my_ppcb = (PPCB*) ppcb;
	if (my_ppcb == NULL)
		return -1;
	
	if (my_ppcb->reader == NULL)
		return -1;
	

	int charsRead = 0;//slash 0
	int buffer_index = 0;
	
	// An einai adeios o buffer && ppcb != null-> kernel_wait sto cv toy has_data
	while ((my_ppcb->free_space == PIPE_BUFFER_SIZE )&&( my_ppcb->writer != NULL) && (my_ppcb->reader != NULL)) 
		kernel_wait(&my_ppcb->has_data, SCHED_PIPE);
	
	

	if (my_ppcb->free_space == PIPE_BUFFER_SIZE)
		return 0;
	else {
		while(my_ppcb->free_space<PIPE_BUFFER_SIZE) {
			if (n > buffer_index+1) {
				buf[buffer_index] = my_ppcb->buffer[my_ppcb->r_position];
				charsRead++;
				buffer_index++;
				my_ppcb ->free_space++;
				my_ppcb->r_position = (my_ppcb->r_position + 1 )% (PIPE_BUFFER_SIZE-1);
			}
			else{
				buf[buffer_index] = '\0';
				charsRead++;
				my_ppcb ->free_space++;
				my_ppcb->r_position = (my_ppcb->r_position + 1 )% (PIPE_BUFFER_SIZE-1);
				break;
			}
		}
	}

	kernel_broadcast(&my_ppcb->has_space);
	return charsRead;
}

// Kano to W toy PPCB null
// Elegxo an R kai W tote free to ppcb
	// kernel_broadcast otan teleioso se kathe periptosi
int pipe_writer_close(void* ppcb){

	PPCB* my_ppcb = (PPCB*) ppcb;

    if (my_ppcb == NULL || my_ppcb->writer == NULL) { // Λάθος όρισμα ή ήδη κλειστό
        return -1;  
    }
	// Αφαιρούμε τον writer
    my_ppcb->writer = NULL;


    if (my_ppcb->reader == NULL) { // Αν και ο reader είναι NULL, απελευθερώνουμε το PPCB
        free(my_ppcb);
        return 0;
    }

    // Εκπέμπουμε σήμα στον reader
	if(my_ppcb->reader != NULL){
    	kernel_broadcast(&my_ppcb->has_data);
	}
    return 0;
}

// Kano to R toy PPCB null
// Elegxo an R kai W tote free to ppcb
// Elegxo an writer einai null den kano broadcast
int pipe_reader_close(void* ppcb){

	PPCB* my_ppcb = (PPCB*) ppcb;

    if (my_ppcb == NULL || my_ppcb->reader == NULL) { // Λάθος όρισμα ή ήδη κλειστό
        return -1;  
    }

	// Αφαιρούμε τον reader
    my_ppcb->reader = NULL;


    if (my_ppcb->writer == NULL) { // Αν και ο writer είναι NULL, απελευθερώνουμε το PPCB
        free(my_ppcb);
        return 0;
    }

    // Εκπέμπουμε σήμα στον writer
	if(my_ppcb->writer != NULL){
    	kernel_broadcast(&(my_ppcb->has_data));
	}
    return 0;
}

// Arikopoiw read kai close
static file_ops reader_file_ops = {
	.Open = invalidFunction2,
	.Write = invalidFunction_write,
	.Read = pipe_read,
	.Close = pipe_reader_close
};

// Arxikopoiw write kai close
static file_ops writer_file_ops = {
	.Open = invalidFunction2,
	.Read = invalidFunction_read,
	.Write = pipe_write,
	.Close = pipe_writer_close
};


int sys_Pipe(pipe_t* pipe)
{
	Fid_t  fid[2];
	FCB* fcb[2];

	if(FCB_reserve(2,fid,fcb)){
		PPCB* ppcb;
		ppcb = xmalloc(sizeof(PPCB));
		ppcb->free_space = PIPE_BUFFER_SIZE;
		ppcb->reader =fcb[0];
		ppcb->writer =fcb[1];
		ppcb->w_position = 0;
		ppcb->r_position = 0;
		ppcb->has_space = COND_INIT;
		ppcb->has_data = COND_INIT;

		pipe->read = fid[0];
		pipe->write = fid[1];


		ppcb->reader->streamobj = ppcb;
		ppcb->reader->streamfunc = &reader_file_ops;

		ppcb->writer->streamobj = ppcb;
		ppcb->writer->streamfunc = &writer_file_ops;

		// reader_file_ops.Read = pipe_read(ppcb, ppcb->buffer, PIPE_BUFFER_SIZE);

		// writer_file_ops.Write = pipe_write(ppcb , ppcb->buffer , PIPE_BUFFER_SIZE);

		// reader_file_ops.Close = pipe_reader_close(ppcb);

		// writer_file_ops.Close = pipe_writer_close(ppcb);

		return 0;
	}
	else
		return -1;
}



