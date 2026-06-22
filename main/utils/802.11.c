#include "utils/frames.h"
#include "utils/settings.h"
#include "utils/802.11.h"
#include <esp_wifi.h>
#include <stdint.h>
#include <string.h>

int send_probe_request(const char* ssid){

    uint8_t ssid_len = strlen(ssid);
    uint8_t payload_len = 2 + ssid_len + 10;
    uint8_t frame_size = sizeof(mac_header_t) + payload_len;

    mac_frame_t* frame = (mac_frame_t*) malloc(frame_size);
    if (frame == NULL)  return 1;

    frame->header.frame_control.protocol_version = 0;
    frame->header.frame_control.type             = 0; 
    frame->header.frame_control.subtype          = 4; // Probe Request    
    frame->header.duration_id = 0x0000;
    memset(frame->header.address1.addr, 0xFF, 6); // Destination: Broadcast
    memcpy(frame->header.address2.addr, &(uint64_t){SPOOF_MAC_ADDRESS}, 6); // Source: SPOOFED MAC
    memset(frame->header.address3.addr, 0xFF, 6); // BSSID: Broadcast
    frame->header.sequence_control.fragment_number = 0;
    frame->header.sequence_control.sequence_number = 0;

    uint32_t p_idx = 0;

    //payload
    //SSID tag
    frame->payload[p_idx++] = 0x00;         // Tag Number
    frame->payload[p_idx++] = ssid_len;     // Tag Length
    memcpy(&frame->payload[p_idx], ssid, ssid_len);
    p_idx += ssid_len;                     

    //supported rates tag 
    frame->payload[p_idx++] = 0x01;         // Tag Number
    frame->payload[p_idx++] = 0x08;         // Tag Length 
    frame->payload[p_idx++] = 0x82;         
    frame->payload[p_idx++] = 0x84;         
    frame->payload[p_idx++] = 0x8B;         
    frame->payload[p_idx++] = 0x96;         
    frame->payload[p_idx++] = 0x0C;         
    frame->payload[p_idx++] = 0x12;         
    frame->payload[p_idx++] = 0x18;         
    frame->payload[p_idx++] = 0x24;         

    //ESP32 driver appends the 4-byte CRC automatically
    esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_STA, (uint8_t*) frame, frame_size, true);
    free(frame);

    if (ret != ESP_OK) {
        printf("ACK tx failed: %s\n", esp_err_to_name(ret));
        return -1;
    }
    return 0;
}


int send_ack(uint8_t* mac) 
{
    ack_frame_t frame; 
    memset(&frame, 0, sizeof(ack_frame_t));
    
    frame.frame_control = 0xD400; 
    frame.duration = 0x0000;
    memcpy(&frame.receiver_address, mac, 6);

    esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_STA, (uint8_t*) &frame, sizeof(ack_frame_t), true);
    if (ret != ESP_OK) {
        printf("ACK tx failed: %s\n", esp_err_to_name(ret));
        return -1;
    }
    return 0;
}