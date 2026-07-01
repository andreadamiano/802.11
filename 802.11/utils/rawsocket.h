#ifndef RAW_SOCKET
#define RAW_SOCKET

#include <stdbool.h>
#include <stdint.h>
#include "utils/settings.h"
#include "utils/frames.h"
#include <pthread.h>
#include <time.h>

struct tag
{
    int key; 
    uint8_t value[256]; 
};

struct filters
{
    mac_header_t header; 
    struct tag tag;  
    // uint8_t destination_mac_address[MAC_LEN];
    // uint8_t source_address[MAC_LEN];
};

typedef struct
{
    int raw_socket; 
    volatile bool running; 
    struct filters filters;
    pthread_mutex_t filter_mutex; 
    pthread_cond_t filter_cond; 
    volatile bool match; 
    struct timespec ts;
}socket_context_t;

extern socket_context_t socket_context; 
extern mac_frame_t filtered_frame; 
extern uint16_t filtered_frame_len;
extern uint8_t spoofed_mac_address[];

int create_rawsocket(int protocol);
int bind_rawsocket(char* ifname, int raw_socket, int protocol);
int set_channel(int raw_socket, const char* ifname, int channel); 
void* listen_mac_frames(void* data);  //producer which append to a circular buffer
void* filter_mac_frames(void* data); //consumer
int send_mac_frame(int raw_socket, mac_frame_t* frame, int frame_len); 
void initialize_socket_context(int raw_socket); 
bool scan_ssid_channel(int raw_socket, const char* ifname, const char* ssid, int* found_channel);
void initialize_filters(); 

#endif