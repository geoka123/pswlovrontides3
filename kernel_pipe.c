#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_pipe.h"
#include "kernel_proc.h"
#include "kernel_dev.h"

// Vale parametrous twn read kai write
int invalidFunction_read(){
	return -1;
}

int invalidFunction_write(){
	return -1;
}
	
void invalidFunction2(){
	return NULL;
}

// Arikopoiw read kai close
static file_ops reader_file_ops = {
	.Open = invalidFunction2,
	.Write = invalidFunction_write
};

// Arxikopoiw write kai close
static file_ops writer_file_ops = {
	.Open = invalidFunction2,
	.Read = invalidFunction_read
};



int sys_Pipe(pipe_t* pipe)
{
	PCB * pcb = CURPROC;

	Fid_t  read = pipe->read;//xrisimopoioyme sosta ta read kai write toy pipe san orisma stin reserve??? episis den exo idea
	Fid_t  write = pipe->write;

	Fid_t  ourbuffer[2];
	ourbuffer[0] = read;
	ourbuffer[1] = write;

	if(FCB_reserve(2,ourbuffer,pcb->FIDT)){
		PPCB* ppcb;
		ppcb = xmalloc(sizeof(PPCB));

		ppcb->w_position =write;
		ppcb->r_position =read;

		ppcb->has_space = COND_INIT;
		ppcb->has_data = COND_INIT;


		get_fcb(read)->streamobj = ppcb;
		get_fcb(read)->streamfunc = &reader_file_ops;



		get_fcb(write)->streamobj = ppcb;
		get_fcb(read)->streamfunc = &writer_file_ops;

		reader_file_ops.Read = pipe_read(ppcb, ppcb->buffer, PIPE_BUFFER_SIZE);

		writer_file_ops.Write = pipe_write(ppcb , ppcb->buffer , PIPE_BUFFER_SIZE);

		reader_file_ops.Close = pipe_reader_close(ppcb);

		writer_file_ops.Close = pipe_writer_close(ppcb);

		return 0;
	}
	else
		return -1;
}

int pipe_write(void* ppcb , const char *buf , unsigned int n){// den exo katalabei ayto to N ti rolo paizei . An prokeitai gia to size toy buffer tote mallon tha prepei na ginei define edw kai oxi sthn kernel_pipe.h
	// ----- ELEGXOI -----
	// Elegxo an ppcb einai null
	// Writer h reader einai closed -> return -1
	// Buffer einai gemato -> kano kernel_wait sti has_space sto while 1) Oso o reader != null kai 2) Oso buffer einai full
	// An bgo apo while kai reader == null -> grafo kai return 0
	// An oxi apo to shmeio pou exo meinei grafo ena ena ta chars mexri na teleiosei
	
	//kano return posa byte egrapsa
	PPCB my_ppcb = (PPCB)ppcb;
	if(my_ppcb == NULL){
		return -1;
	}

	if(my_ppcb->reader==NULL || my_ppcb->writer == NULL){
		return -1;
	}

	

	while(my_ppcb->reader != NULL && my_ppcb->w_position ){ // edo ligo thema
		kernel_wait(&(ppcb->has_space)); 
	}

	if(my_ppcb->reader == NULL){
		return -1;
	}

	int freeSpace = (PIPE_BUFFER_SIZE - 1) - ppcb->w_position;
	int charsWritten = 0;
	int buffer_index = 0;

	
	while(freeSpace >=0){
		if(n<=buffer_index-1){
			my_ppcb->buffer[my_ppcb->w_position] = buf[buffer_index];
			charsWritten++;
			buffer_index++;
			my_ppcb->w_position = (my_ppcb->w_position + 1 )% (PIPE_BUFFER_SIZE-1);
		}
		else{
			break;
		}
	}
	
	kernel_broadcast(&(ppcb->has_data));
	return charsWritten;
}

int pipe_read(void* ppcb ,char *buf , unsigned int n){
	// ----- ELEGXOI -----
	// An reader
	// Elegxo an ppcb einai null
	// An einai adeios o buffer && ppcb != null-> kernel_wait sto cv toy has_data
	// Elegxo an writer einai null -> diavase
	// Elegxo an o buffer exei dedomena
	return -1;
}

// Kano to W toy PPCB null
// Elegxo an R kai W tote free to ppcb
	// kernel_broadcast otan teleioso se kathe periptosi
int pipe_writer_close(void* ppcb){
	return -1;
}

// Kano to R toy PPCB null
// Elegxo an R kai W tote free to ppcb
// Elegxo an writer einai null den kano broadcast
int pipe_reader_close(void* ppcb){
	return -1;
}

