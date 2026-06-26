#include <stdbool.h>


int create_rawsocket(int protocol);
int bind_rawsocket(char* ifname, int raw_socket, int protocol);