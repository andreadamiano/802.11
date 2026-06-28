#include "utils/rawsocket.h"
#include "utils/settings.h"
#include "utils/802.11.h"
#include "utils/frames.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/ethernet.h>
#include <linux/sockios.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <memory.h>
#include <stdio.h>
#include <linux/wireless.h>
#include <pthread.h>
#include <unistd.h>


int create_rawsocket(int protocol)
{
    int raw_socket;
    if ((raw_socket = socket(PF_PACKET, SOCK_RAW, htons(protocol))) == -1)
    {
        perror("Creating raw socket"); 
    }
    return raw_socket;
}


int bind_rawsocket(char* ifname, int raw_socket, int protocol)
{
    //raw sockets bind to a network interface (bybpassing the enitre tcp-ip stack)
    struct sockaddr_ll sll; //struct holding the layer 2 address
    struct ifreq ifr; //interface request needed for ioctl request

    memset(&sll, 0, sizeof(struct sockaddr_ll));
    memset(&ifr, 0, sizeof(struct ifreq));

    //get network interface index
    strncpy( (char*) ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(raw_socket, SIOCGIFINDEX, &ifr) == -1)
    {
        perror("Getting interface inde");
        return -1; 
    }

    //bind socket to the network interface
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(protocol);
    sll.sll_ifindex = ifr.ifr_ifindex;

    if(bind(raw_socket, (struct sockaddr*) &sll, sizeof(sll)) == -1)
    {
        perror("Binding raw socket");
        return -1; 
    }

    return 1; 
    
}

int set_channel(int raw_socket, const char* ifname, int channel)
{
    struct iwreq iwreq; //struct used to receive/set configuration on wireless devices via ioctl

    memset(&iwreq, 0, sizeof(iwreq)); 

    //initialize iwreq structure
    strncpy(iwreq.ifr_name, ifname, strlen(ifname));
    iwreq.u.freq.m = channel; 
    iwreq.u.freq.e = 0; 
    iwreq.u.freq.flags = IW_FREQ_FIXED; 

    //sending request using the configured struct 
    if (ioctl(raw_socket, SIOCSIWFREQ, &iwreq) == -1)
    {
        perror("While setting network interface channel");
        return -1; 
    }

    return 0; 
}


void* listen_mac_frames(void* data)
{
    int* raw_socket = ((int *) data); 
    uint8_t buffer[MAC_FRAME_SIZE]; //fine tune the buffer size to make mac frames fit inside
    ssize_t bytes; 
    printf("started new thread %ld\n", (unsigned long) pthread_self());
    while (true)
    {
        bytes = read(*raw_socket, buffer, sizeof(buffer)); 
        if (bytes > 0 )
        {
            print_frame(buffer, bytes);
        }
        else
        {
            perror("While reading from raw socket");
            break;
        }
    }

    return NULL;
}