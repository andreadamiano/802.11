#ifndef RAW_SOCKET
#define RAW_SOCKET

#include <stdbool.h>


int create_rawsocket(int protocol);
int bind_rawsocket(char* ifname, int raw_socket, int protocol);
int set_channel(int raw_socket, const char* ifname, int channel); 
void* listen_mac_frames(void* data);

#endif