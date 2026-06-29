#ifndef FRAME_QUEUE
#define FRAME_QUEUE

#include "utils/frames.h"
#include "utils/settings.h"
#include <pthread.h>

typedef struct queue_node
{
    mac_frame_t frame; 
    uint16_t frame_len; 
    struct queue_node* next; 
}queue_node;

typedef struct 
{
    uint16_t head;
    uint16_t tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    queue_node nodes[FRAME_QUEUE_SIZE];  //queue ring
}frame_queue_t;

void enqueue_frame(mac_frame_t* frame, uint16_t frame_len); 
queue_node* dequeue_frame(); 


#endif