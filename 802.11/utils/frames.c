#include "utils/frames.h"
#include <stdio.h>
#include <stdbool.h>


uint8_t get_fixed_params_length(frame_control_t fc) 
{
    // We only process Management Frames (Type == 0)
    if (fc.type != 0) {
        return 0; 
    }

    switch (fc.subtype) {
        case 0:  // Association Request
            return 4;   
            
        case 1:  // Association Response
        case 3:  // Reassociation Response
            return 6;   
            
        case 2:  // Reassociation Request
            return 10;  
            
        case 4:  // Probe Request
        case 9:  // ATIM Frame
            return 0;   
            
        case 5:  // Probe Response
        case 8:  // Beacon Frame
            return 12;  
            
        case 10: // Disassociation
        case 12: // Deauthentication
            return 2;   
            
        case 11: // Authentication
            return 6;   
            
        case 13: // Action Frame
            return 0; 

        default:
            return 0;   
    }
}

void print_frame(mac_frame_t* frame, uint8_t frame_len)
{
    if (frame == NULL) {
        printf("Error: Frame pointer is NULL\n");
        return;
    }

    frame_control_t fc = frame->header.frame_control;

    printf("============ FRAME CONTROL ============\n");
    printf("Protocol Version : %u\n", fc.protocol_version);
    printf("Frame Type       : %u\n", fc.type);
    printf("Frame Subtype    : %u\n", fc.subtype);
    printf("To DS            : %u\n", fc.to_ds);
    printf("From DS          : %u\n", fc.from_ds);
    printf("More Fragments   : %u\n", fc.more_fragments);
    printf("Retry            : %u\n", fc.retry);
    printf("Power Management : %u\n", fc.power_mgmt);
    printf("More Data        : %u\n", fc.more_data);
    printf("Protected Frame  : %u\n", fc.protected_frame);
    printf("Order Bit        : %u\n", fc.order);
    
    printf("============ ADDRESS FIELDS ============\n");
    printf("Address 1: %02X:%02X:%02X:%02X:%02X:%02X\n",
           frame->header.address1.addr[0], frame->header.address1.addr[1],
           frame->header.address1.addr[2], frame->header.address1.addr[3],
           frame->header.address1.addr[4], frame->header.address1.addr[5]);
           
    printf("Address 2: %02X:%02X:%02X:%02X:%02X:%02X\n",
           frame->header.address2.addr[0], frame->header.address2.addr[1],
           frame->header.address2.addr[2], frame->header.address2.addr[3],
           frame->header.address2.addr[4], frame->header.address2.addr[5]);

    printf("Address 3: %02X:%02X:%02X:%02X:%02X:%02X\n",
           frame->header.address3.addr[0], frame->header.address3.addr[1],
           frame->header.address3.addr[2], frame->header.address3.addr[3],
           frame->header.address3.addr[4], frame->header.address3.addr[5]);

    printf("============ PAYLOAD ============\n");
    
    uint8_t fixed_params_length = get_fixed_params_length(frame->header.frame_control);
    uint8_t* ch = frame->payload + fixed_params_length;
    uint8_t current_byte = 24 + fixed_params_length;  //the header is 24 bytes long + 12 bytes of probe response paramters
    frame_len -= 4; //subtract the CRC field

    while (current_byte < frame_len)
    {
        uint8_t tag_number = *(ch++);
        uint8_t tag_length = *(ch++);
        printf("Tag:  %02X\n", tag_number);
        printf("Content length:  %d\n", tag_length);
        printf("Content: ");
        for(int i =0; i < tag_length; ++i,  ++ch)   printf("%02X", *ch);
        printf("\n");
        current_byte += tag_length + 2;
    }
    printf("============ END OF FRAME ============\n\n");
}

int16_t get_tag(mac_frame_t* frame, uint8_t frame_len, uint8_t tag, uint8_t** content)
{
    int8_t fixed_params_length = get_fixed_params_length(frame->header.frame_control);
    uint8_t* ch = frame->payload + fixed_params_length;
    uint8_t current_byte = 24 + fixed_params_length;  //the header is 24 bytes long + 12 bytes of probe response paramters
    frame_len -= 4; //subtract the CRC field

    while (current_byte < frame_len)
    {
        uint8_t tag_number = *(ch++);
        uint8_t tag_length = *(ch++);

        if (tag_number == tag)
        {
            *content = ch; 
            return tag_length;
        }

        ch += tag_length;
        current_byte += tag_length + 2;
    }

    *content = NULL;
    return -1;
}