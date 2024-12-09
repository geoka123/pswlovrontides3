#ifndef __KERNEL_SOCKET_H
#define __KERNEL_SOCKET_H



#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_proc.h"
#include "kernel_pipe.h"



typedef enum socket_type_e {
  SOCKET_LISTENER,   /**< @brief The PID is free and available */
  SOCKET_UNBOUND,  /**< @brief The PID is given to a process */
  SOCKET_PEER  /**< @brief The PID is held by a zombie */
}socket_type;




typedef struct listener_socket{
    rlnode queue;
    CondVar req_available;
}listener_socket;

typedef struct peer_socket{
    SCCB* peer;
    PPCB* write_pipe;
    PPCB* read_pipe;
}peer_socket;

typedef struct unbound_socket{
    rlnode unbound;
}unbound_socket;

typedef struct socket_control_block{
    unsigned int refcount;
    FCB* fcb;
    socket_type type;
    port_t port;

    union{
        listener_socket listener_s;
        unbound_socket unbound_s;
        peer_socket peer_s;
    };
}SCCB;



typedef struct connection_request{
    int admitted;
    SCCB* peer;
    CondVar connected_cv;
    rlnode queue_node;
}connection_request;



#endif