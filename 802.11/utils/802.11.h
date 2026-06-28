#ifndef PROTOCOL_802_11
#define PROTOCOL_802_11

#include <stdint.h>
#include "utils/frames.h"

int send_probe_request(const char* ssid);
void print_frame(uint8_t* buffer, uint16_t buffer_len); 
int16_t get_tag(mac_frame_t* frame, uint16_t frame_len, uint8_t tag, uint8_t** content);  

#endif