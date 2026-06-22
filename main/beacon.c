#include <stdio.h>
#include <esp_wifi.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

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

    //raw hex packet
    uint8_t packet[] = {
        /* Frame Control: Beacon */
        0x80, 0x00,
        /* Duration */
        0x00, 0x00,
        /* Destination: broadcast */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        /* Source MAC */
        0x02, 0x02, 0x03, 0x04, 0x05, 0x06,
        /* BSSID */
        0x02, 0x02, 0x03, 0x04, 0x05, 0x06,
        /* Sequence Control */
        0xc0, 0x6c,
        /* Timestamp (8 bytes) */
        0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00,
        /* Beacon Interval: 100 TU */
        0x64, 0x00,
        /* Capability Info: ESS + Short Preamble */
        0x11, 0x04,   // <-- was 0x00,0x04 (missing ESS bit!)

        /* IE: SSID = "rrrrrr" */
        0x00, 0x06, 'r', 'r', 'r', 'r', 'r', 'r',

        /* IE: Supported Rates */
        0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,

        /* IE: DS Parameter Set (channel 4) */
        0x03, 0x01, 0x04,

        /* IE: TIM (required by iOS) */
        0x05, 0x04, 0x00, 0x01, 0x00, 0x00
    };

    //start WiFi driver
    printf("Starting WiFI driver\n");
    ret = esp_wifi_start();
    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_set_channel(4, WIFI_SECOND_CHAN_NONE);
    ESP_ERROR_CHECK(ret);

    uint64_t timestamp = 0;
    printf("About to send packets every 200ms\n");
    while(true)
    {
        memcpy(&packet[24], &timestamp, 8);
        timestamp += 102400; // increment by 100 TU (in microseconds)
        esp_wifi_80211_tx(WIFI_IF_STA, packet, sizeof(packet), true);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
