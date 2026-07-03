#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <linux/if_ether.h> 
#include <unistd.h>
#include <pthread.h>
#include "utils/settings.h"
#include "utils/802.11.h"
#include "utils/rawsocket.h"
#include "utils/frames.h"
#include <stdlib.h>


//global states
char ifname[16]; //name of the network interface
unsigned char local_mac[MAC_LEN];
unsigned char bssid[MAC_LEN];
char ssid[16];
int ret; 
pthread_t listening_mac_thread_id; 
int raw_socket;
mac_frame_t* mac_frame; 


int main (int argc, char* argv[])
{
    mac_frame_t* response;
    uint16_t response_len;
    int ssid_channel;

    if (argc < 3)
    {
	    fprintf(stderr,"Usage: %s <ifname> <ssid>\n", argv[0]);
	    return -1;
    }

    //parse command line arguments
    strncpy(ifname, argv[1], strlen(argv[1]));
    strncpy(ssid, argv[2], strlen(argv[2]));

    if ((raw_socket = create_rawsocket(ETH_P_ALL)) == -1)
    {
        return -1;
    }

    if (bind_rawsocket(ifname, raw_socket, ETH_P_ALL) == -1)
    {
        return -1;
    }

    //launch a background thread listening for incoming mac frames in monitor mode
    initialize_socket_context(raw_socket);

    if (pthread_create(&listening_mac_thread_id, NULL, &listen_mac_frames, NULL) != 0)
    {
        perror("Creating listening thread"); 
        return -1; 
    }

    //scan channels to find the provided ssid
    if (!scan_ssid_channel(raw_socket, ifname, ssid, &ssid_channel))
    {
        printf("SSID channel not found");
        return -1;
    }
    
    if (!send_probe_request_to_ssid_with_response(raw_socket, ssid, &response, &response_len))
    {
        perror("Sending probe request"); 
        return -1; 
    }

    //retrieve the bssid of the AP
    memcpy(&bssid, &response->header.address2.addr, MAC_LEN);

    if (!send_authentication_to_bssid_with_response(raw_socket, bssid, &response, &response_len))
    {
        perror("Sending authentication request"); 
        return -1; 
    }
    

    //block main thread execution
    if (pthread_join(listening_mac_thread_id, NULL) != 0)
    {
        perror("While joining listening thread"); 
        return -1; 
    }
    
    close(raw_socket); 
    return 0; 
}