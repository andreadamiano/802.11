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
    memcpy(&new_node->frame, frame, sizeof(mac_frame_t));
    new_node->frame_len = frame_len;

    pthread_cond_signal(&frame_queue.cond);
    pthread_mutex_unlock(&frame_queue.mutex);
    
}

queue_node* dequeue_frame()
{
    if (frame_queue.head == frame_queue.tail)
        return NULL; 

    queue_node* node;

    //lock access to the shared resource
    pthread_mutex_lock(&frame_queue.mutex);
    
    node = &(frame_queue.nodes[(frame_queue.head++ % FRAME_QUEUE_SIZE)]);

    pthread_cond_signal(&frame_queue.cond);
    pthread_mutex_unlock(&frame_queue.mutex);
}