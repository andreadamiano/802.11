#ifndef PROTOCOL_802_11
#define PROTOCOL_802_11

#include <stdint.h>
#include <stdbool.h>
#include "utils/frames.h"
#include "utils/rawsocket.h"

extern mac_frame_t* current_frame; 
extern uint16_t current_frame_len;
extern bool has_fcs;

int send_probe_request_to_ssid(int raw_socket, const char* ssid);
bool send_probe_request_to_ssid_with_response(int raw_socket, const char* ssid, mac_frame_t** response, uint16_t* response_len);
int send_authentication_to_bssid(int raw_socket, const char* bssid);
int send_authentication_to_bssid_with_response(int raw_socket, const char* bssid, mac_frame_t** response, uint16_t* response_len);
void print_frame(mac_frame_t* frame, uint16_t frame_len); 
void parse_frame(uint8_t* buffer, uint16_t buffer_len); 
int16_t get_tag(mac_frame_t* frame, uint16_t frame_len, uint8_t tag, uint8_t** content);  
bool filter_frame(mac_frame_t* frame, uint16_t frame_len, struct filters* filters);
int channel_to_freq(int channel);

#endif