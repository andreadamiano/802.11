#define _POSIX_C_SOURCE 199309L
#include "utils/frames.h"
#include "utils/settings.h"
#include "utils/802.11.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils/rawsocket.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

mac_frame_t* current_frame; 
uint16_t current_frame_len; 
bool has_fcs;

int send_probe_request_to_ssid(int raw_socket, const char* ssid){

    uint8_t ssid_len = strlen(ssid);
    uint8_t payload_len = 2 + ssid_len + 10;
    uint16_t frame_size = sizeof(mac_header_t) + payload_len;
    uint16_t bytes; 
    mac_frame_t frame;
    
    memset(&frame, 0, sizeof(mac_frame_t)); //zero initialize

    frame.header.frame_control.protocol_version = 0;
    frame.header.frame_control.type             = 0; //management frame
    frame.header.frame_control.subtype          = 4; //probe Request    
    frame.header.duration_id = 0x0000;
    memset(frame.header.address1.addr, 0xFF, MAC_LEN); //destination: broadcast
    memcpy(frame.header.address2.addr, spoofed_mac_address, MAC_LEN); //source: spoofed MAC
    memset(frame.header.address3.addr, 0xFF, MAC_LEN); //final destination: broadcast
    frame.header.sequence_control.fragment_number = 0;
    frame.header.sequence_control.sequence_number = 0;

    uint32_t frame_index = 0;

    //payload
    //SSID tag
    frame.payload[frame_index++] = 0x00;         //tag number
    frame.payload[frame_index++] = ssid_len;     //tag length
    memcpy(&frame.payload[frame_index], ssid, ssid_len);
    frame_index += ssid_len;                     

    //supported rates tag 
    frame.payload[frame_index++] = 0x01;         //tag number
    frame.payload[frame_index++] = 0x08;         //tag length 
    frame.payload[frame_index++] = 0x82;         
    frame.payload[frame_index++] = 0x84;         
    frame.payload[frame_index++] = 0x8B;         
    frame.payload[frame_index++] = 0x96;         
    frame.payload[frame_index++] = 0x0C;         
    frame.payload[frame_index++] = 0x12;         
    frame.payload[frame_index++] = 0x18;         
    frame.payload[frame_index++] = 0x24;         

    //send mac frame
    bytes = send_mac_frame(raw_socket, &frame, frame_size);     
    return bytes;
}


uint8_t get_fixed_params_length(frame_control_t fc) 
{
    // We only process Management Frames (Type == 0)
    if (fc.type != 0) {
        return 0; 
    }

    switch (fc.subtype) {
        case 0:  //association request
            return 4;   
            
        case 1:  //association response
        case 3:  //reassociation response
            return 6;   
            
        case 2:  //reassociation request
            return 10;  
            
        case 4:  //probe request
        case 9:  //ATIM frame
            return 0;   
            
        case 5:  //probe response
        case 8:  //beacon frame
            return 12;  
            
        case 10: //disassociation
        case 12: //deauthentication
            return 2;   
            
        case 11: //authentication
            return 6;   
            
        case 13: //action frame
            return 0; 

        default:
            return 0;   
    }
}

void print_frame(mac_frame_t* frame, uint16_t frame_len)
{

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

    if (frame->header.frame_control.type == 0)
    {
        
        uint8_t fixed_params_length = get_fixed_params_length(frame->header.frame_control);
        uint8_t* ch = frame->payload + fixed_params_length; //the header is 24 bytes long + 12 bytes of probe response paramters
        uint8_t* frame_end = (uint8_t* )frame + frame_len; //get an hard boundary (to avoid that corrupted mac frames can make the program read past the buffer)
        
        //subtract the CRC field
        if (frame_len > 4 && has_fcs) {
            frame_end -= 4; 
        }

        if (fixed_params_length > 0)
        {
            printf("--------- FIXED PARAMETERS ---------\n");
            
            if (frame->header.frame_control.subtype == 11) //authentication frame
            {
                uint16_t algo = *(uint16_t*)&frame->payload[0];
                uint16_t seq  = *(uint16_t*)&frame->payload[2];
                uint16_t stat = *(uint16_t*)&frame->payload[4];
                
                printf("Auth Algorithm : %d\n", algo);
                printf("Auth Sequence  : %d\n", seq);
                printf("Auth Status    : %d\n", stat);
            }
            else if (frame->header.frame_control.subtype == 5 || frame->header.frame_control.subtype == 8) //probe response or beacon frame
            {
                uint64_t timestamp = *(uint64_t*)&frame->payload[0];
                uint16_t interval  = *(uint16_t*)&frame->payload[8];
                uint16_t cap_info  = *(uint16_t*)&frame->payload[10];
                
                printf("Timestamp      : %lu\n", timestamp);
                printf("Beacon Interval: %d\n", interval);
                printf("Capability Info: 0x%04X\n", cap_info);
            }
            else
            {
                //fallback to raw hex dump
                printf("Raw Bytes (%d): ", fixed_params_length);
                for (int i = 0; i < fixed_params_length; i++) {
                    printf("%02X ", frame->payload[i]);
                }
                printf("\n");
            }
            printf("-------------------------------------\n");
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
                printf("============ END OF FRAME ============\n\n");
                return; 
            }

            for(int i =0; i < tag_length; ++i,  ++ch)
            {
                printf("%02X", *ch);
            }
            printf("\n");
        }
    }
    else
    {
        //control frames and data frames do not have TLV structure (raw hexdump)
        uint8_t* ch = frame->payload;
        uint8_t* frame_end = (uint8_t* )frame + frame_len; //get an hard boundary (to avoid that corrupted mac frames can make the program read past the buffer)

        //subtract the CRC field
        if (frame_len > 4 && has_fcs) {
            frame_end -= 4; 
        }

        while(ch < frame_end)
        {
            printf("%02X", *(ch++));
        }
        printf("\n");
    }
    printf("============ END OF FRAME ============\n\n");
}

int16_t get_tag(mac_frame_t* frame, uint16_t frame_len, uint8_t tag, uint8_t** content)
{
    int8_t fixed_params_length = get_fixed_params_length(frame->header.frame_control);
    uint8_t* ch = frame->payload + fixed_params_length;
    uint8_t* frame_end = (uint8_t* )frame + frame_len;

    //subtract the CRC field
    if (frame_len > 4 && has_fcs)
    {
        frame_end -= 4; 
    }

    while (ch + 2 < frame_end)
    {
        uint8_t tag_number = *(ch++);
        uint8_t tag_length = *(ch++);

        if (ch + tag_length > frame_end)
        {
            return -1; 
        }

        if (tag_number == tag)
        {
            *content = ch; 
            return tag_length;
        }

        ch += tag_length;
    }

    *content = NULL;
    return -1;
}

void parse_frame(uint8_t* buffer, uint16_t buffer_len)
{
    if (buffer == NULL || buffer_len < 8) {
        return;
    }

    //handle the radiotap header
    uint16_t radiotap_len = *(uint16_t *)(buffer + 2);
    if (radiotap_len >= buffer_len) {
        return; 
    }

    radiotap_header_t* rt_header = (radiotap_header_t*) buffer;
    uint8_t* p = (uint8_t*)rt_header + 8; //go past to the fixed part of the radiotap header
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
    uint32_t* current_it_present =(uint32_t*) (&rt_header->it_present);
    #pragma GCC diagnostic pop
    
    //if 31 bit is set to one there could be more it_present fields
    while (*current_it_present & (1U <<31))
    {
        current_it_present +=1;
        p += 4;
    }

    //check if TSFT is present (signaled by bit 0 of the it_present filed)
    if (rt_header->it_present & 1)
    {   
        uintptr_t off = p - (uint8_t*)rt_header;
        off = (off + 7) & ~7;              // round up to next multiple of 8
        p = (uint8_t*)rt_header + off;

        p += 8;  //skip the TSFT data
    }

    if (rt_header->it_present & (1 << 1)) //flag field is present
    {         
        if (p < buffer + radiotap_len) 
        {  
            uint8_t flags = *p;
            has_fcs = (flags & 0x10) != 0; //bit 4 signal if the CRC is present or not
        }
    }

    current_frame = (mac_frame_t*)(buffer + radiotap_len);
    current_frame_len = buffer_len - radiotap_len;

}

bool filter_frame(mac_frame_t* frame, uint16_t frame_len, struct filters* filters)
{
    uint8_t* content = NULL; 
    int content_len = get_tag(frame, frame_len, filters->tag.key, &content);
    bool result = false; 

    if (filters->header.address1.addr[0] == 0xff || (strncmp(frame->header.address1.addr, filters->header.address1.addr, 6) == 0))
    {
        result = true;
    }
    else
    {
        return false; 
    }

    if (filters->header.address2.addr[0] == 0xff || (strncmp(frame->header.address2.addr, filters->header.address2.addr, 6) == 0))
    {
        result = true;
    }
    else
    {
        return false; 
    }

    if ( *((uint16_t*) &filters->header.frame_control) == 0xffff  || (*((uint16_t*) &frame->header.frame_control) == *((uint16_t*) &filters->header.frame_control)))
    {
        result = true;
    }
    else
    {
        return false; 
    }

    if (filters->tag.key == -1  || (content_len > 0 && content != NULL && (strncmp(content, filters->tag.value, content_len) == 0)))
    {
        result = true;
    }
    else
    {
        return false; 
    }

    return result; 
}

bool send_probe_request_to_ssid_with_response(int raw_socket, const char* ssid, mac_frame_t** response, uint16_t* response_len)
{
    pthread_mutex_lock(&socket_context.filter_mutex);

    // define the filters to catch the response
    initialize_filters();
    socket_context.filters.tag.key = 0; 
    strncpy(socket_context.filters.tag.value, ssid, strlen(ssid));
    socket_context.filters.header.frame_control.subtype = 5; 
    pthread_mutex_unlock(&socket_context.filter_mutex);

    send_probe_request_to_ssid(raw_socket, ssid);

    pthread_mutex_lock(&socket_context.filter_mutex);

    //initialize timeout
    clock_gettime(CLOCK_REALTIME, &socket_context.ts); 
    socket_context.ts.tv_sec += 2;

    while (!socket_context.match)
    {
        //wait for a conditional with a timeout 
        int rc = pthread_cond_timedwait(&socket_context.filter_cond, &socket_context.filter_mutex, &socket_context.ts);
    
        if (rc == ETIMEDOUT)
        {
            socket_context.match = false;
            pthread_mutex_unlock(&socket_context.filter_mutex);
            return false;
        }
    }
    *response = &filtered_frame; //zero copy
    *response_len = filtered_frame_len;
    socket_context.match = false; //reset flag
    pthread_mutex_unlock(&socket_context.filter_mutex);

    //debug
    printf("Response to probe request:\n");
    print_frame(*response, *response_len);
    
    return true;
}

int send_authentication_to_bssid(int raw_socket, const char* bssid)
{
    uint16_t frame_size = sizeof(mac_header_t) + 6;
    uint16_t bytes; 
    mac_frame_t frame;
    
    memset(&frame, 0, sizeof(mac_frame_t)); //zero initialize

    frame.header.frame_control.protocol_version = 0;
    frame.header.frame_control.type             = 0; //management frame
    frame.header.frame_control.subtype          = 11; //authentication request
    frame.header.duration_id = 0x0000;
    memcpy(frame.header.address1.addr, bssid, MAC_LEN); //destination: bssid
    memcpy(frame.header.address2.addr, spoofed_mac_address, MAC_LEN); //source: spoofed MAC
    memcpy(frame.header.address3.addr, bssid, MAC_LEN); //final destination: broadcast
    frame.header.sequence_control.fragment_number = 0;
    frame.header.sequence_control.sequence_number = 0;

    uint8_t payload_index = 0;

    //algorithm
    frame.payload[payload_index++] = 0;
    frame.payload[payload_index++] = 0;

    //sequence
    frame.payload[payload_index++] = 1; //little endian order
    frame.payload[payload_index++] = 0;

    //status code
    frame.payload[payload_index++] = 0;
    frame.payload[payload_index++] = 0;

    //send mac frame
    bytes = send_mac_frame(raw_socket, &frame, frame_size);     
    return bytes;
}

int send_authentication_to_bssid_with_response(int raw_socket, const char* bssid, mac_frame_t** response, uint16_t* response_len)
{
    pthread_mutex_lock(&socket_context.filter_mutex);

    // define the filters to catch the response
    initialize_filters();
    socket_context.filters.header.frame_control.subtype = 11;
    memcpy(socket_context.filters.header.address1.addr, spoofed_mac_address, MAC_LEN); 
    memcpy(socket_context.filters.header.address2.addr, bssid, MAC_LEN); 

    pthread_mutex_unlock(&socket_context.filter_mutex);

    send_authentication_to_bssid(raw_socket, bssid);

    pthread_mutex_lock(&socket_context.filter_mutex);

    //initialize timeout
    clock_gettime(CLOCK_REALTIME, &socket_context.ts); 
    socket_context.ts.tv_sec += 2;

    while (!socket_context.match)
    {
        //wait for a conditional with a timeout 
        int rc = pthread_cond_timedwait(&socket_context.filter_cond, &socket_context.filter_mutex, &socket_context.ts);
    
        if (rc == ETIMEDOUT)
        {
            socket_context.match = false;
            pthread_mutex_unlock(&socket_context.filter_mutex);
            return false;
        }
    }
    *response = &filtered_frame; //zero copy
    *response_len = filtered_frame_len;
    socket_context.match = false; //reset flag
    pthread_mutex_unlock(&socket_context.filter_mutex);

    //debug
    printf("Response to authentication request:\n");
    print_frame(*response, *response_len);
    
    return true;
}