#ifndef PROTOCOL_802_11
#define PROTOCOL_802_11

#include <stdint.h>
#include <stdbool.h>
#include "utils/frames.h"
#include "utils/rawsocket.h"

extern mac_frame_t* current_frame; 
extern uint16_t current_frame_len;

int send_probe_request(int raw_socket, const char* ssid);
void print_current_frame(); 
void parse_frame(uint8_t* buffer, uint16_t buffer_len); 
int16_t get_tag(mac_frame_t* frame, uint16_t frame_len, uint8_t tag, uint8_t** content);  
bool filter_current_frame(struct filters* filters);

#endif