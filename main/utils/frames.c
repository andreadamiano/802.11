#include "utils/frames.h"
#include <stdio.h>
#include <stdbool.h>

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
    
    uint8_t* ch = frame->payload;
    uint8_t current_byte = 24;  //the header is 24 bytes long

    while (current_byte < frame_len)
    {
        printf("Tag:  %02X\n", *(ch++));

        uint8_t* tag_length = (uint8_t*) ch;
        printf("Content length:  %d\n", *tag_length);
        printf("Content: ");
        for(int i =0; i < *tag_length; ++i,  ++ch)   printf("%02X", *ch);
        printf("\n");
        current_byte += *tag_length + 2;
    }
    printf("============ END OF FRAME ============\n");
    printf("\n\n");
}