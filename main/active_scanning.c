#include <stdio.h>
#include <esp_wifi.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/frames.h"
#include <string.h>
#include <stdio.h>

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
    memset(frame->header.address2.addr, 0x02, 6); // Source: Invented MAC
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

    //little endian format
    // uint8_t packet[] = {
    //     0x40, 0x00,                                                             // Frame Control (Probe Request)
    //     0x00, 0x00,                                                             // Duration (0 ms)
    //     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                                     // Destination (Broadcast) 
    //     0x02, 0x02, 0x03, 0x04, 0x05, 0x06,                                     // Source (invented MAC)
    //     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                                     // BSSID: (Broadcast)
    //     0x00, 0x00,                                                             // Sequence Control

    //     //frame body (key, lenght, value information in the probe request)
    //     0x00,                                                                   // Tag Number: 0x00 (SSID)
    //     0x0A,                                                                   // Tag Length: 10 bytes (0x0A in hex)
    //     'F', 'A', 'S', 'T', 'W', 'E', 'B', '-', 'D', '5',  // access point SSID

    //     0x01,                                                                   // Tag Number: 0x01 (Supported Rates)
    //     0x08,                                                                   // Tag Length: 4 bytes
    //     0x82, 0x84, 0x8B, 0x96, 0x0C, 0x12, 0x18, 0x24                          // Tag Value: Supported speeds
    // };

    //ESP32 driver appends the 4-byte CRC automatically
    esp_wifi_80211_tx(WIFI_IF_STA, (uint8_t*) frame, frame_size, true);
    free(frame);
    return 0;
}


void process_packet(void* buffer, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *wifi_packet = (wifi_promiscuous_pkt_t *) buffer; 

    //parse the header
    mac_frame_t* frame = (mac_frame_t*) wifi_packet->payload;
    
    if (frame->header.frame_control.type == 0 && frame->header.frame_control.subtype == 5)
    {
        //print packet
        print_frame(frame, (uint8_t) wifi_packet->rx_ctrl.sig_len);
    }


}


void app_main(void)
{
    esp_err_t ret;

    //initialize NVS
    ret = nvs_flash_init();
    ESP_ERROR_CHECK(ret);

    //initialize TCP-IP stack 
    ret = esp_netif_init();
    ESP_ERROR_CHECK(ret);

    //start event loop
    ret = esp_event_loop_create_default();
    ESP_ERROR_CHECK(ret);

    //create default WiFi station network instance (it acts as a translator between the network stack and the Tcp-Ip stack)
    esp_netif_create_default_wifi_sta();

    //initialize WiFi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    ret = esp_wifi_init(&cfg); 
    ESP_ERROR_CHECK(ret);

    //configure WI-FI driver
    printf("Setting up WiFI driver...\n");
    ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_set_promiscuous(true);  
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_set_promiscuous_rx_cb(process_packet); //register a callback 
    ESP_ERROR_CHECK(ret);

    send_probe_request("FASTWEB-D5");

}