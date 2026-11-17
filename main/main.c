#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <string.h>

// Define the GPIO pin for the LED (GPIO 2 is common for onboard LEDs)
#define BLINK_GPIO 21

void getMAC(uint8_t *mac) {
    esp_err_t macret = esp_wifi_get_mac(WIFI_IF_STA, mac);
    if (macret == ESP_OK) {
        printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        printf("Failed to get MAC address\n");
    } 

}

typedef struct msgStruct {
    char a[32];
    int b; //length of the data
} msgStruct;

void app_main(void)
{
    //set up nvs flash!
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { // if error, clear it and start over
        ESP_ERROR_CHECK(nvs_flash_erase()); 
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); // do error handling

    

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //set up wifi config with default values
    printf(esp_wifi_init(&cfg) == ESP_OK ? "WIFI SUCCESS\n" : "WIFI FAIL\n"); //initalize wifi
    printf(esp_now_init() == ESP_OK ? "ESP-NOW SUCCESS\n" : "ESP-NOW FAIL\n"); //initaolize esp-now


    uint8_t HostMAC[6] = {0}; //make a buffer to hold the mac address 
    getMAC(HostMAC); //put the mac address in the buffer
    printf("Host MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", HostMAC[0], HostMAC[1], HostMAC[2], HostMAC[3], HostMAC[4], HostMAC[5]);

    
    //MAC:44:BD:8D:76:A1:F0 with ant
    //MAC: 44:BD:8D:76:A2:F4 w/o ant


    uint8_t broadcastAddress[] = {0x44, 0xBD, 0x8D, 0x76, 0xA1, 0xF0}; //MAC address of the receiver
    esp_now_peer_info_t peerInfo; //store info about the peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6); //copy the mac address into the peer info struct
    peerInfo.channel = 0; //use channel 0
    peerInfo.encrypt = false; //no encryption

    while (esp_now_add_peer(&peerInfo) != ESP_OK){ //add the peer, if it fials then escape
        printf("Failed to add peer");
    }


    msgStruct packet; //make a packet to send!

    strcpy(packet.a, "Hi Ryan, This is the other guy"); //some text idk
    packet.b = strlen(packet.a); //length of the text, might as well send that ig

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &packet, sizeof(packet)); //send the packet with MAC addr, the packet, and the length of that whole packet

    while (1) {
        result = esp_now_send(broadcastAddress, (uint8_t *) &packet, sizeof(packet));
        if (result == ESP_OK) { //check if it sent
            printf("Sent with success\n");
        } else {
            printf("Error sending the data: %d\n", result);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); //wait a second
    }




}