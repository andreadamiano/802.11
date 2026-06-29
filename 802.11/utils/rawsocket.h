#ifndef RAW_SOCKET
#define RAW_SOCKET

#include <stdbool.h>
#include <stdint.h>
#include "utils/settings.h"
#include "utils/frames.h"

struct tag
{
    int key; 
    uint8_t value[256]; 
};

struct filters
{
    mac_header_t header; 
    struct tag tag;  
    uint8_t destination_mac_address[MAC_LEN];
    uint8_t source_address[MAC_LEN];
};

typedef struct
{
    int raw_socket; 
    struct filters filters;
}socket_context_t;


int create_rawsocket(int protocol);
int bind_rawsocket(char* ifname, int raw_socket, int protocol);
int set_channel(int raw_socket, const char* ifname, int channel); 
void* listen_mac_frames(void* data);

#endif