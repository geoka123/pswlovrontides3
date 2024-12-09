
#include "tinyos.h"
#include "kernel_socket.h"
#include "kernel_streams.h"
#include "kernel_pipe.h"
#include "kernel_proc.h"
 
#include "kernel_dev.h"
#include "kernel_cc.h" 

SCCB* PORT_MAP[MAX_PORT];
//PORT_MAP[0] = NULL;

void* socket_invalidFunction_open(){
	return NULL;
}

int socket_read(void* sccb , char* buf , unsigned int size){
	if(sccb!=NULL){
		SCCB* my_sccb = (SCCB*)sccb;
		if(my_sccb->type == SOCKET_PEER){
			peer_socket* peer = &my_sccb->peer_s;
			return pipe_read(peer->read_pipe,buf,size);
		}
		return -1;
	}
	return -1;
}

int socket_write(void* sccb , const char *buf , unsigned int n){
	if(sccb!=NULL){
		SCCB* my_sccb = (SCCB*)sccb;
		if(my_sccb->type == SOCKET_PEER){
			peer_socket* peer = &my_sccb->peer_s;
			return pipe_write(peer->write_pipe,buf,n);
		}
		return -1;
	}
	return -1;
}

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
	if(port>=NOPORT && port <= MAX_PORT){	
		if(FCB_reserve(1,&fid,&fcb)){
			SCCB* sccb;
			sccb = xmalloc(sizeof(SCCB));
			sccb->refcount = 0;
			sccb->type = SOCKET_UNBOUND; //unbound
			if(port!=NOPORT)
				sccb->port = port;
			else
				sccb->port = NOPORT;
			fcb->streamfunc = &socket_file_ops;
			fcb->streamobj = sccb;
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
	if(sock >= 0 && sock <= MAX_FILEID -1){
		FCB* my_sock = (FCB*)&sock;
		if(my_sock->streamobj!= NULL){
			SCCB* sccb = (SCCB*)my_sock->streamobj;
			if(sccb->type == SOCKET_PEER){
				switch(how){
					case SHUTDOWN_READ:
						peer_socket* peer = &sccb->peer_s;
						return pipe_reader_close(peer->read_pipe);
					break;

					case SHUTDOWN_WRITE:
						peer_socket* peer2 = &sccb->peer_s;
						return pipe_writer_close(peer2->write_pipe);
					break;

					case SHUTDOWN_BOTH:
						peer_socket* peer3 = &sccb->peer_s;
						int j = pipe_writer_close(peer3->write_pipe);
						int i = pipe_reader_close(peer3->read_pipe);
						if(i==j){
							return i;
						}
						return -1;
					break;

					default:
						return -1;
				}
				return 0;
			}
			return -1;
		}
		return -1;
	}
	return -1;
}

