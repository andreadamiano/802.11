#include "utils/frame_queue.h"
#include <memory.h>

static frame_queue_t frame_queue = {0}; 

void enqueue_frame(mac_frame_t* frame, uint16_t frame_len)
{

    if (frame == NULL)
        return;

    //lock access to the shared resource
    pthread_mutex_lock(&frame_queue.mutex);
    struct queue_node* new_node; 

    //check if the queue is full
    if (frame_queue.head - frame_queue.tail == FRAME_QUEUE_SIZE) //counter increments infinitely (in C unsigned subtractions are always computed modulo 2^N, where N is the nummber of bits of the operands)
    {
        pthread_mutex_unlock(&frame_queue.mutex);
        return;
    }
    
    new_node = &frame_queue.nodes[(frame_queue.head++ % FRAME_QUEUE_SIZE)];
    memset(&new_node->frame, 0, sizeof(mac_frame_t)); //remove garbage values from previous enqueues
    memcpy(&new_node->frame, frame, frame_len); //copy only actual data
    new_node->frame_len = frame_len;

    pthread_cond_signal(&frame_queue.cond);
    pthread_mutex_unlock(&frame_queue.mutex);
    
}

bool dequeue_frame(mac_frame_t* frame, uint16_t* frame_len)
{
    pthread_mutex_lock(&frame_queue.mutex); //the mutex must be grab before waiting on a conditional variable in order to avoid the lost wakeup problem

    while ((frame_queue.head - frame_queue.tail) == 0) //the while is used to prevent spurious wake ups
    {
        pthread_cond_wait(&frame_queue.cond, &frame_queue.mutex); //cond wait atomically release the lock and put the thread to sleep
    }

    struct queue_node* node = &(frame_queue.nodes[(frame_queue.tail++ % FRAME_QUEUE_SIZE)]);

    //copy the content of the queue node
    memcpy(frame, &node->frame, sizeof(mac_frame_t)); 
    *frame_len = node->frame_len;
    pthread_mutex_unlock(&frame_queue.mutex);
    return true; 
}