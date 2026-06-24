#include <stdio.h>
#include <esp_wifi.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/frames.h"
#include "utils/settings.h"
#include "utils/802.11.h"
#include <string.h>
#include <stdio.h>

//global states
volatile bool probe_response_received = false;
uint8_t access_point_mac[6];
mac_frame_t received_frame[MAX_FRAME_SIZE];
uint16_t received_frame_len; 
uint8_t ap_channel;
bool promiscuous_mode = true;

void process_packet(void* buffer, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *wifi_packet = (wifi_promiscuous_pkt_t *) buffer; 

    //parse the MAC frame
    mac_frame_t* frame = (mac_frame_t*) wifi_packet->payload;
    uint16_t frame_len = (uint16_t) wifi_packet->rx_ctrl.sig_len;
    
    if (frame->header.frame_control.type == 0 && frame->header.frame_control.subtype == 5)
    {

        //get ssid
        uint8_t* content = NULL; 
        int content_len;
        content_len = get_tag(frame, frame_len, 0x00, &content);

        if (content_len && (memcmp((char*) content, SSID, (uint8_t) strlen(SSID)) == 0) && (memcmp(frame->header.address1.addr, &(uint64_t){SPOOF_MAC_ADDRESS}, 6) == 0))
        {
            //save global states
            printf("Operating on Channel: %d\n", wifi_packet->rx_ctrl.channel);
            print_frame(frame, frame_len);

            memcpy(received_frame, frame, frame_len);
            received_frame_len = frame_len;
            ap_channel = wifi_packet->rx_ctrl.channel;
            memcpy(access_point_mac, received_frame->header.address2.addr, 6);
            probe_response_received = true;
        }
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

    ret = esp_wifi_set_promiscuous(promiscuous_mode);  
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_set_promiscuous_rx_cb(process_packet); //register a callback 
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_set_mac(WIFI_IF_STA, (const uint8_t*)&(uint64_t){SPOOF_MAC_ADDRESS}); //by settings the mac address, the esp32 will automatically send the ack message upon receiving the frame
    ESP_ERROR_CHECK(ret);

    printf("Starting WiFI driver\n");
    ret = esp_wifi_start();
    ESP_ERROR_CHECK(ret);

    esp_wifi_set_channel(2, WIFI_SECOND_CHAN_NONE);
    send_probe_request(SSID);

    // uint8_t current_channel = 1;
    // while(!probe_response_received)
    // {
    //     vTaskDelay(pdMS_TO_TICKS(200));
    //     esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
    //     send_probe_request(SSID);

    //     current_channel++;
    //     if (current_channel > 13) {
    //         current_channel = 1;
    //     }
    // }

    
    // printf("Operating on Channel: %d\n", ap_channel);
    // print_frame(received_frame, received_frame_len);

    
}



