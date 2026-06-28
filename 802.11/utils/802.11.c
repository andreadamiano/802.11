#include "utils/frames.h"
// #include "utils/settings.h"
// #include "utils/802.11.h"
// #include <stdint.h>
// #include <string.h>
// #include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// int send_probe_request(const char* ssid){

//     uint8_t ssid_len = strlen(ssid);
//     uint8_t payload_len = 2 + ssid_len + 10;
//     uint8_t frame_size = sizeof(mac_header_t) + payload_len;

//     mac_frame_t* frame = (mac_frame_t*) malloc(frame_size);
//     if (frame == NULL)  return 1;

//     memset(frame, 0, frame_size); //always zero initialize heap allocated structs

//     frame->header.frame_control.protocol_version = 0;
//     frame->header.frame_control.type             = 0; 
//     frame->header.frame_control.subtype          = 4; // Probe Request    
//     frame->header.duration_id = 0x0000;
//     memset(frame->header.address1.addr, 0xFF, 6); // Destination: Broadcast
//     memcpy(frame->header.address2.addr, &(uint64_t){SPOOF_MAC_ADDRESS}, 6); // Source: SPOOFED MAC
//     memset(frame->header.address3.addr, 0xFF, 6); // BSSID: Broadcast
//     frame->header.sequence_control.fragment_number = 0;
//     frame->header.sequence_control.sequence_number = 0;

//     uint32_t frame_index = 0;

//     //payload
//     //SSID tag
//     frame->payload[frame_index++] = 0x00;         // Tag Number
//     frame->payload[frame_index++] = ssid_len;     // Tag Length
//     memcpy(&frame->payload[frame_index], ssid, ssid_len);
//     frame_index += ssid_len;                     

//     //supported rates tag 
//     frame->payload[frame_index++] = 0x01;         // Tag Number
//     frame->payload[frame_index++] = 0x08;         // Tag Length 
//     frame->payload[frame_index++] = 0x82;         
//     frame->payload[frame_index++] = 0x84;         
//     frame->payload[frame_index++] = 0x8B;         
//     frame->payload[frame_index++] = 0x96;         
//     frame->payload[frame_index++] = 0x0C;         
//     frame->payload[frame_index++] = 0x12;         
//     frame->payload[frame_index++] = 0x18;         
//     frame->payload[frame_index++] = 0x24;         

//     //ESP32 driver appends the 4-byte CRC automatically
//     esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_STA, (uint8_t*) frame, frame_size, true);
//     free(frame);

//     if (ret != ESP_OK) {
//         printf("ACK tx failed: %s\n", esp_err_to_name(ret));
//         return -1;
//     }
//     return 0;
// }


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

void print_frame(uint8_t* buffer, uint16_t buffer_len)
{
    if (buffer == NULL || buffer_len < 4) {
        return;
    }

    //handle the radiotap header
    uint16_t radiotap_len = *(uint16_t *)(buffer + 2);
    if (radiotap_len >= buffer_len) {
        return; 
    }

    mac_frame_t* frame = (mac_frame_t*)(buffer + radiotap_len);
    uint16_t frame_len = buffer_len - radiotap_len;

    //parse the 802.11 header
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
    uint8_t* ch = frame->payload + fixed_params_length; //the header is 24 bytes long + 12 bytes of probe response paramters
    uint8_t* frame_end = (uint8_t* )frame + frame_len - 4; //get an hard boundary (to avoid that corrupted mac frames can make the program read past the buffer)
    
    //subtract the CRC field
    if (frame_len > 4) {
        frame_len -= 4; 
    }

    while (ch + 2 <= frame_end)
    {
        uint8_t tag_number = *(ch++);
        uint8_t tag_length = *(ch++);

        printf("Tag:  %02X\n", tag_number);
        printf("Content length:  %d\n", tag_length);
        printf("Content: ");
        
        //prune early if a malformed tag arrives
        if (ch + tag_length > frame_end)
        {
            printf("<truncated>\n"); 
            return; 
        }

        for(int i =0; i < tag_length; ++i,  ++ch)
        {
            printf("%02X", *ch);
        }
        printf("\n");
    }
    printf("============ END OF FRAME ============\n\n");
}

int16_t get_tag(mac_frame_t* frame, uint16_t frame_len, uint8_t tag, uint8_t** content)
{
    int8_t fixed_params_length = get_fixed_params_length(frame->header.frame_control);
    uint8_t* ch = frame->payload + fixed_params_length;
    uint8_t current_byte = 24 + fixed_params_length;  //the header is 24 bytes long + 12 bytes of probe response paramters

    //subtract the CRC field
    if (frame_len > 4)
    {
        frame_len -= 4; 
    }

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