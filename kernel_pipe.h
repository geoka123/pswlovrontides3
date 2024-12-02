#ifndef __KERNEL_PIPE_H
#define __KERNEL_PIPE_H



#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_proc.h"
#define PIPE_BUFFER_SIZE 8192

typedef struct pipe_control_block{
    FCB *reader,*writer;
    CondVar has_space;
    CondVar has_data;

    int w_position , r_position;
    int free_space;
    char buffer[PIPE_BUFFER_SIZE];

}PPCB;


#endif