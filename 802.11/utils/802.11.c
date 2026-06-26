// #include "utils/frames.h"
// #include "utils/settings.h"
// #include "utils/802.11.h"
// #include <stdint.h>
// #include <string.h>
// #include <stdlib.h>

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
