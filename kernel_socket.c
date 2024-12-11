#include "tinyos.h"
#include "kernel_socket.h"
#include "kernel_streams.h"
#include "kernel_proc.h"

//#include "kernel_dev.h"
#include "kernel_cc.h" 

SCCB* PORT_MAP[MAX_PORT];
//PORT_MAP[0] = NULL;

void initialize_sockets() {
	for (int i=0; i<MAX_PORT; i++) {
		PORT_MAP[i] = NULL;
	}
}

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
int socket_close(void* sccb){
	// If a listener is being closed => kernel_broadcast to Accept
	if(sccb!= NULL){
		SCCB* my_sccb = (SCCB*)sccb;
		
		FCB* my_fcb = my_sccb->fcb;
		if(my_fcb==NULL){
			return -1;
		}
		//SCCB* my_sock =(SCCB*)my_fcb->streamobj;
		for(int i = 0;i< MAX_FILEID; i++){
			if(CURPROC->FIDT[i] == my_fcb){
				Fid_t fid = (Fid_t)i;
				if(sys_ShutDown(fid,SHUTDOWN_BOTH)!=0){
					return -1;
				}
			}
		}

		//GIATI DEN DOULEUEI???
		// if (my_sccb->type == SOCKET_LISTENER)
		// 		kernel_broadcast(&my_sccb->listener_s.req_available);

		if(my_sccb->refcount == 0){
			PORT_MAP[my_sccb->port]=NULL;
			//my_fcb->streamobj = NULL;
			my_sccb = NULL;
			free(my_sccb);
			return -1;
		}
		return 0;
	}
	return -1;
}

static file_ops socket_file_ops = {
	.Open = socket_invalidFunction_open,
	.Read = socket_read,
	.Write = socket_write,
	.Close = socket_close
};


Fid_t sys_Socket(port_t port)
{
	Fid_t fid;
	FCB* fcb;
	if((port>=NOPORT && port <= MAX_PORT)){
		if(FCB_reserve(1,&fid,&fcb)){
			SCCB* sccb;
			sccb = (SCCB*)xmalloc(sizeof(SCCB));
			sccb->refcount = 0;
			sccb->type = SOCKET_UNBOUND; //unbound
			if(port>NOPORT)
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
	if(sock == NOFILE)
		return -1;

	FCB* fcb_sock = get_fcb(sock);
	
	if (fcb_sock == NULL)
		return -1;
	SCCB* my_sock = (SCCB*) fcb_sock->streamobj;

	if (my_sock == NULL || my_sock->port == NOPORT)
		return -1;

	if (PORT_MAP[my_sock->port] != NULL)
		return -1;

	// Install the socket into PORT_MAP[]
	PORT_MAP[my_sock->port] = my_sock;
	
	// Mark the socket as a listener socket
	my_sock->type = SOCKET_LISTENER;

	rlnode_init(&my_sock->listener_s.queue, my_sock);
	my_sock->listener_s.req_available = COND_INIT;

	return 0;
}

Fid_t sys_Accept(Fid_t lsock)
{
	// ---------- ELEGXOI ----------
	if(lsock <= NOFILE || lsock >= MAX_FILEID)
		return -1;
	
	FCB* fcb_of_sock = get_fcb(lsock);

	if (fcb_of_sock == NULL)
		return -1;
	
	SCCB* my_sock = (SCCB*) fcb_of_sock->streamobj;

	if (my_sock == NULL || my_sock->port == NOPORT || my_sock->type == SOCKET_UNBOUND)
		return NOFILE;
	// How do i check if listening socket was closed while waiting

	// Increase refcount
	my_sock->refcount++;

	while(is_rlist_empty(&my_sock->listener_s.queue) != 0) {
		kernel_wait(&my_sock->listener_s.req_available, SCHED_USER);
	}
	//my_sock->refcount--;
	// Check if port is still valid
	if (PORT_MAP[my_sock->port] == NULL)
		return NOFILE;
	
	// Honor first request of queue
	connection_request* first_request = (connection_request*) rlist_pop_front(&my_sock->listener_s.queue)->obj;  //Mhpws de ginetai swsta to casting??
	if(first_request == NULL)
		return NOFILE;

	// Try to construct peer    APO POU PAIRNO TA PPCB KAI AN TO KANO SOSTA
	//my_sock->type = SOCKET_PEER;
	SCCB* my_sock2 = first_request->peer; //s2	
	my_sock2->type = SOCKET_PEER;
	
	Fid_t my_sock_fid3 = sys_Socket(my_sock->port); //s3 ///edo ti ginetai charge9
	if(my_sock_fid3 == NOFILE)
		return NOFILE;

	FCB* fcb_of_sock3 = get_fcb(my_sock_fid3);
	if(fcb_of_sock3->streamobj == NULL)
		return NOFILE;
	SCCB* my_sock3 = (SCCB*) fcb_of_sock3->streamobj;
	if(my_sock3 == NULL)
		return NOFILE;
	my_sock3->type = SOCKET_PEER;

	// Ftiaxno PPCB* me xmalloc
	PPCB* pipe1 = (PPCB*)xmalloc(sizeof(PPCB));
	PPCB* pipe2 = (PPCB*)xmalloc(sizeof(PPCB));
	
	//pipe initialization 

	// Ftiaxno ta peer to peer
	my_sock2->peer_s.peer = my_sock3;
	my_sock3->peer_s.peer = my_sock2;

	my_sock2->peer_s.read_pipe->reader = pipe1->reader;
	my_sock3->peer_s.read_pipe->reader = pipe2->reader;

	my_sock2->peer_s.write_pipe->writer = pipe2->writer;
	my_sock3->peer_s.write_pipe->writer = pipe1->writer;

	first_request->admitted = 1;
	kernel_signal(&(first_request->connected_cv));
	my_sock->refcount--;

	return my_sock_fid3;
}

int sys_Connect(Fid_t sock, port_t port, timeout_t timeout)
{
	// if(sock < 0 || sock > MAX_FILEID -1){
	// 	return -1;
	// }

	if(sock == NOFILE){
		return -1;
	}
	FCB * my_sock = get_fcb(sock);

	if(my_sock->streamobj==NULL){
		return -1;
	}

	SCCB* sccb = (SCCB*)my_sock->streamobj;//s2 client

	if(!(port>=NOPORT && port <= MAX_PORT)){
		return -1;
	}

	if(PORT_MAP[port]==NULL){
		return -1;
	}

	SCCB* is_listener = PORT_MAP[port];//s1 listener

	if(!(is_listener->type == SOCKET_LISTENER)){
		return -1;
	}

	is_listener->refcount++;
	sccb->type = SOCKET_PEER;

	//sccb->type = SOCKET_PEER;//charge 
	connection_request* con_req = (connection_request*)xmalloc(sizeof(connection_request));

	con_req -> admitted = 0;
	con_req ->peer = sccb;
	con_req->connected_cv = COND_INIT;

	rlnode_init(&con_req->queue_node,con_req);

	rlist_push_back(&is_listener->listener_s.queue,&con_req->queue_node);


	kernel_signal(&(is_listener->listener_s.req_available));

	while(con_req ->admitted == 0 ){
		kernel_timedwait(&(con_req->connected_cv),SCHED_USER,timeout*1000);
	}
	if(con_req->admitted==1){
		is_listener->refcount--;
		return 0;
	}
	is_listener->refcount--;
	return -1;
}

int sys_ShutDown(Fid_t sock, shutdown_mode how)
{
	// if(sock >= 0 && sock <= MAX_FILEID -1){
	// 	FCB* my_sock = (FCB*)&sock;
	// 	if(my_sock->streamobj!= NULL){
	// 		SCCB* sccb = (SCCB*)my_sock->streamobj;
	// 		if(sccb->type == SOCKET_PEER){
	// 			switch(how){
	// 				case SHUTDOWN_READ:
	// 					peer_socket* peer = &sccb->peer_s;
	// 					return pipe_reader_close(peer->read_pipe);
	// 				break;

	// 				case SHUTDOWN_WRITE:
	// 					peer_socket* peer2 = &sccb->peer_s;
	// 					return pipe_writer_close(peer2->write_pipe);
	// 				break;

	// 				case SHUTDOWN_BOTH:
	// 					peer_socket* peer3 = &sccb->peer_s;
	// 					int j = pipe_writer_close(peer3->write_pipe);
	// 					int i = pipe_reader_close(peer3->read_pipe);
	// 					if(i==j){
	// 						return i;
	// 					}
	// 					return -1;
	// 				break;

	// 				default:
	// 					return -1;
	// 			}
	// 			//return 0;
	// 		}
	// 		return -1;
	// 	}
	// 	return -1;
	// }
	// return -1;
	if(!(sock >= 0 && sock <= MAX_FILEID -1)){
		return -1;
	}

	FCB* my_sock = get_fcb(sock);
	if(my_sock->streamobj== NULL){
		return -1;
	}
	SCCB* sccb = (SCCB*)my_sock->streamobj;
	if(sccb == NULL)
		return -1;
	if(sccb->type != SOCKET_PEER){
		return -1;
	}
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

}

