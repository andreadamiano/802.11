#include <stdio.h>
#include <esp_wifi.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/frames.h"

void process_packet(void* buffer, wifi_promiscuous_pkt_type_t type)
{
    uint8_t* ch; 
    wifi_promiscuous_pkt_t *wifi_packet = (wifi_promiscuous_pkt_t *) buffer; 
    ch = wifi_packet->payload;
    
    frame_control_t* frame_id = (frame_control_t*) ch;

    //print packet
    printf("Received packet: ");
    for (int i=0; i < wifi_packet->rx_ctrl.sig_len; ++i, ++ch) printf("%02X", *ch);
    printf("\n\n");
    printf("%02X", frame_id->type);
    printf("\n\n");
    printf("%02X", frame_id->subtype);
    printf("\n\n");

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

    //start WiFi driver
    printf("Starting WiFI driver\n");
    ret = esp_wifi_start();
    ESP_ERROR_CHECK(ret);

    uint8_t current_channel = 1;
    printf("Start scanning WiFi Packets\n");
    while(true)
    {
        esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
        
        //loop over all 13 channels
        current_channel++;
        if (current_channel > 13) {
            current_channel = 1;
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
