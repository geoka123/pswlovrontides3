
#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_pipe.h"
#include "kernel_proc.h"
#include "kernel_dev.h"

static file_ops reader_file_ops;
reader_file_ops->Open = invalidFunction2(); //thelei asteraki sto Open??????den exo idea
reader_file_ops->Write = invalidFunction1();


static file_ops writer_file_ops;
writer_file_ops->Open = invalidFunction2();
writer_file_ops->Read= invalidFunction1();



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

		ppcb->w_position =read;
		ppcb->r_position =write;

		ppcb->has_space = COND_INIT;
		ppcb->has_data = COND_INIT;


		get_fcb(read)->streamobj = ppcb;
		get_fcb(read)->streamfunc = writer_file_ops;



		get_fcb(write)->streamobj = ppcb;
		get_fcb(read)->streamfunc = reader_file_ops;

		reader_file_ops->Read = pipe_read(ppcb, ppcb->buffer, n!!!);

		writer_file_ops->Write = pipe_write(ppcb , ppcb->buffer , n!!!);

		reader_file_ops->Close = pipe_reader_close(ppcb);

		writer_file_ops->Close = pipe_writer_close(ppcb);

		return 0;
	}
	return -1;


	int invalidFunction1(){
		return -1;
	}
	void invalidFunction2(){
		return NULL;
	}



	int pipe_write(void* ppcb , const char *buf , unsigned int n){// den exo katalabei ayto to N ti rolo paizei . An prokeitai gia to size toy buffer tote mallon tha prepei na ginei define edw kai oxi sthn kernel_pipe.h
		return -1
	}

	int pipe_read(void* ppcb ,char *buf , unsigned int n){
		return -1
	}

	int pipe_writer_close(void* ppcb){
		return -1;
	}

	int pipe_reader_close(void* ppcb){
		return -1;
	}
}

