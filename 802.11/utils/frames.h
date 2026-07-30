#ifndef FRAMES_H
#define FRAMES_H

#include <stdint.h>
#include "utils/settings.h"

//bits are filled from right to left  (starting at the least significant bit)
typedef struct {
    uint16_t protocol_version : 2; 
    uint16_t type             : 2; 
    uint16_t subtype          : 4; 
    uint16_t to_ds            : 1; 
    uint16_t from_ds          : 1;
    uint16_t more_fragments   : 1;
    uint16_t retry            : 1; 
    uint16_t power_mgmt       : 1; 
    uint16_t more_data        : 1; 
    uint16_t protected_frame  : 1; 
    uint16_t order            : 1; 
} __attribute__((packed)) frame_control_t;


typedef struct {
    uint16_t fragment_number : 4;  
    uint16_t sequence_number : 12; 
} __attribute__((packed)) sequence_control_t;

typedef struct {
    uint8_t addr[6];
} mac_address_t;

typedef struct {
    frame_control_t    frame_control;    
    uint16_t           duration_id;       
    mac_address_t      address1;          
    mac_address_t      address2;        
    mac_address_t      address3;          
    sequence_control_t sequence_control;  
} __attribute__((packed)) mac_header_t;

typedef struct {
    mac_header_t header; 
    uint8_t payload[MAX_FRAME_PAYLOAD]; 
} __attribute__((packed)) mac_frame_t;

typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    mac_address_t receiver_address; 
} __attribute__((packed)) ack_frame_t;

typedef struct {
    uint8_t  it_version;     
    uint8_t  it_pad;         
    uint16_t it_len;         
    uint32_t it_present;     
} __attribute__((packed)) radiotap_header_t;

typedef struct {
    uint8_t  dsap;       
    uint8_t  ssap;       
    uint8_t  control;    
    uint8_t  oui[3];     
    uint16_t ethertype;  
} __attribute__((packed)) llc_snap_header_t;

typedef struct {
    uint8_t  version;             
    uint8_t  packet_type;         
    uint16_t packet_body_length;  
} __attribute__((packed)) eapol_header_t;

typedef struct {
    uint8_t  key_descriptor_type; 
    uint16_t key_info;            
    uint16_t key_length;          
    uint64_t replay_counter;      
    uint8_t  key_nonce[32];       
    uint8_t  key_iv[16];          
    uint8_t  key_rsc[8];          
    uint8_t  reserved[8];         
    uint8_t  key_mic[16];         
    uint16_t key_data_length;     
    uint8_t  key_data[];          
} __attribute__((packed)) eapol_key_frame_t;

typedef struct {
    llc_snap_header_t snap;
    eapol_header_t    eapol;
    eapol_key_frame_t key_frame;
} __attribute__((packed)) complete_eapol_frame_t;

#endif