#include <stdint.h>

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
    uint8_t payload[]; 
} __attribute__((packed)) mac_frame_t;

typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    mac_address_t receiver_address; // Receiver Address
    uint8_t fcs[4]; 
    uint8_t _pad[2];  
} __attribute__((packed)) ack_frame_t;


void print_frame(mac_frame_t* frame, uint8_t frame_len); 
int16_t get_tag(mac_frame_t* frame, uint8_t frame_len, uint8_t tag, uint8_t** content);  