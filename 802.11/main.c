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
unsigned int local_mac[MAC_LEN];
unsigned int remote_mac[MAC_LEN];
char ssid[16];
int ret; 
pthread_t listening_mac_thread_id; 
int raw_socket;
mac_frame_t* mac_frame; 


int main (int argc, char* argv[])
{

    if (argc < 4)
    {
	    fprintf(stderr,"Usage: %s <ifname> <remote_mac> <ssid>\n", argv[0]);
	    return -1;
    }

    //parse command line arguments
    strncpy(ifname, argv[1], strlen(argv[1]));

    ret = sscanf(argv[2], "%02x:%02x:%02x:%02x:%02x:%02x", &remote_mac[0], &remote_mac[1], &remote_mac[2], &remote_mac[3], &remote_mac[4], &remote_mac[5]);
    if (ret < 6)
    {
        perror("Remote mac MUST be in human readable form like 01:02:03:04:05:06\r\n");
        return -1;
    }

    strncpy(ssid, argv[3], strlen(argv[3]));


    if ((raw_socket = create_rawsocket(ETH_P_ALL)) == -1)
    {
        return -1;
    }

    if (bind_rawsocket(ifname, raw_socket, ETH_P_ALL) == -1)
    {
        return -1;
    }

    //launch a background thread listening for incoming mac frames in monitor mode
    if (pthread_create(&listening_mac_thread_id, NULL, &listen_mac_frames, (void*) &raw_socket) != 0)
    {
        perror("Creating listening thread"); 
        return -1; 
    }
    
    while (true)
    {
        sleep(1); 
    }
    





}