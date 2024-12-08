
#include "tinyos.h"
#include "kernel_socket.h"
#include "kernel_streams.h"
#include "kernel_pipe.h"
#include "kernel_proc.h"
#include "kernel_dev.h"
#include "kernel_cc.h" 

SCCB* PORT_MAP[MAX_PORT];
PORT_MAP[0] = NOPORT;

void* socket_invalidFunction_open(){
	return NULL;
}

int socket_read(void* sccb , char* buf , unsigned int size){}

int socket_write(void* sccb , const char *buf , unsigned int n){}

int socket_close(void* sccb){}

static file_ops socket_file_ops = {
  .Open = socket_invalidFunction_open,
  .Read = socket_read,
  .Write = socket_write,
  .Close = socket_close,
};


Fid_t sys_Socket(port_t port)
{
	Fid_t fid;
	FCB* fcb;
	if(port>NOPORT && port < MAX_PORT -1){	
		if(FCB_reserve(1,fid,fcb)){
			SCCB* sccb;
			sccb = xmalloc(sizeof(SSCB));
			sccb->refcount = 0;
			sccb->type = 1; //unbound

			sccb->port = port;


			fcb->streamfunc = &socket_file_ops;
			fcb->streamobj = sscb;
			return fid;
		}
	}
	return NOFILE;
}

int sys_Listen(Fid_t sock)
{
	return -1;
}


Fid_t sys_Accept(Fid_t lsock)
{
	return NOFILE;
}


int sys_Connect(Fid_t sock, port_t port, timeout_t timeout)
{
	return -1;
}


int sys_ShutDown(Fid_t sock, shutdown_mode how)
{
	return -1;
}

