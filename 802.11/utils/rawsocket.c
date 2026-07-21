#include "utils/rawsocket.h"
#include "utils/settings.h"
#include "utils/802.11.h"
#include "utils/frames.h"
#include "utils/frame_queue.h"
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
#include <sys/uio.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <unistd.h>

socket_context_t socket_context;  //-1 will be considered a default so it will not be filtered
mac_frame_t filtered_frame; 
uint16_t filtered_frame_len;
uint8_t spoofed_mac_address[MAC_LEN] = {SPOOF_MAC_ADDRESS};

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
        perror("Getting interface index");
        return -1; 
    }

    //save interface index
    socket_context.if_index = ifr.ifr_ifindex;

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
    // struct iwreq iwreq; //struct used to receive/set configuration on wireless devices via ioctl

    // memset(&iwreq, 0, sizeof(iwreq)); 

    // //initialize iwreq structure
    // strncpy(iwreq.ifr_name, ifname, strlen(ifname));
    // iwreq.u.freq.m = channel; 
    // iwreq.u.freq.e = 0; 
    // iwreq.u.freq.flags = IW_FREQ_FIXED; 

    // //sending request using the configured struct 
    // if (ioctl(raw_socket, SIOCSIWFREQ, &iwreq) == -1)
    // {
    //     perror("Setting network interface channel");
    //     // return -1; 
    // }

    // return 0; 

    int frequency_mhz = channel_to_freq(channel);

    if (frequency_mhz == 0)
    {
        perror("Inavalid wifi channel");
        return -1;
    }

    //create and connect Netlink socket
    //Netlink socket are used as a form of interporcess communication with the kernel space
    //over this sockets messages will be sent to the kernel, which will communicate with the NIC, using the appropriate driver
    struct nl_sock* nl_socket = nl_socket_alloc();

    if (!nl_socket)
    {
        perror("Allocating Netlink socket");
        return -1; 
    }

    if (genl_connect(nl_socket) < 0)
    {
        perror("Connecting to Netlink socket");
        return -1; 
    }

    //resolve to which subsystem ID we want our socket to comunicate
    int family_id = genl_ctrl_resolve(nl_socket, "nl80211");
    if (family_id < 0) 
    {
        fprintf(stderr, "nl80211 interface not found in kernel\n");
        nl_socket_free(nl_socket);
        return -1;
    }

    //allocate nl message
    struct nl_msg *nl_message = nlmsg_alloc();
    if (!nl_message)
    {
        perror("Allocating nl message");
        nl_socket_free(nl_socket);
        return -1; 
    }

    //assemble command header
    genlmsg_put(nl_message, 0, 0, genl_ctrl_resolve(nl_socket, "nl80211"), 0, 0, NL80211_CMD_SET_WIPHY, 0);

    //compone message to change the frequency of the NIC
    NLA_PUT_U32(nl_message, NL80211_ATTR_IFINDEX, socket_context.if_index);
    NLA_PUT_U32(nl_message, NL80211_ATTR_WIPHY_FREQ, frequency_mhz);
    NLA_PUT_U32(nl_message, NL80211_ATTR_WIPHY_CHANNEL_TYPE, NL80211_CHAN_NO_HT);

    //send message
    int ret = nl_send_auto_complete(nl_socket, nl_message);
    if (ret < 0)
    {
        perror("Sending Netlink message");
        return -1; 
    }
    

    nlmsg_free(nl_message);
    nl_socket_free(nl_socket);
    return 0;

    nla_put_failure:
        nlmsg_free(nl_message);
        nl_socket_free(nl_socket);
        printf("PUT Failure\n");
        return 1;

}


void* listen_mac_frames(void* data)
{
    // socket_context_t* context = (socket_context_t *) data; 
    uint8_t buffer[MAC_FRAME_SIZE]; //fine tune the buffer size to make mac frames fit inside the buffer
    ssize_t bytes; 
    pthread_t filtering_mac_thread_id; 

    //start consumer
    if (pthread_create(&filtering_mac_thread_id, NULL, &filter_mac_frames, NULL) != 0)
    {
        perror("Creating filtering thread"); 
        exit(-1); 
    }

    printf("Started listening thread with ID: %ld\n", (unsigned long) pthread_self());
    while (socket_context.running)
    {
        bytes = read(socket_context.raw_socket, buffer, sizeof(buffer)); 
        if (bytes > 0)
        {
            parse_frame(buffer, bytes); 
            enqueue_frame(current_frame, current_frame_len);
        }
        else
        {
            perror("While reading from raw socket");
            exit(-1);
        }
    }

    if (pthread_join(filtering_mac_thread_id, NULL) != 0)
    {
        perror("While joining filtering thread"); 
        exit(-1);
    }

    return NULL;
}


int send_mac_frame(int raw_socket, mac_frame_t* frame, int frame_len)
{
    uint16_t bytes; 
    radiotap_header_t rt_header;
    struct iovec iovecs[2]; 

    //add radiotap header before sending raw mac frame
    memset(&rt_header, 0, sizeof(radiotap_header_t));
    rt_header.it_version = 0;
    rt_header.it_pad     = 0;
    rt_header.it_len     = sizeof(radiotap_header_t); 
    rt_header.it_present = 0;

    iovecs[0].iov_base = &rt_header; 
    iovecs[0].iov_len = sizeof(radiotap_header_t);
    iovecs[1].iov_base = frame;
    iovecs[1].iov_len = frame_len;


    if ((bytes = writev(raw_socket, iovecs, 2)) != (frame_len + sizeof(radiotap_header_t)))
    {
        return -1; 
    }
    return bytes; 
}

void* filter_mac_frames(void* data)
{
    // socket_context_t* context = (socket_context_t* )data; 
    mac_frame_t current_dequeued_frame;
    uint16_t frame_len;

    printf("Started filtering thread with ID: %ld\n", (unsigned long) pthread_self());
    while(socket_context.running)
    {
        if (dequeue_frame(&current_dequeued_frame, &frame_len))
        {
                //grab a mutex while reading the current filters (which can be modified asynchronously)
                pthread_mutex_lock(&socket_context.filter_mutex);
                if (filter_frame(&current_dequeued_frame, frame_len, &socket_context.filters))
                {
                    memset(&filtered_frame, 0, sizeof(mac_frame_t));
                    memcpy(&filtered_frame, &current_dequeued_frame, frame_len);
                    filtered_frame_len = frame_len;
                    
                    //signal to other threads that a match has been found
                    socket_context.match = true;
                    pthread_cond_signal(&socket_context.filter_cond);
                    // print_frame(&current_dequeued_frame, frame_len); 
                }
                pthread_mutex_unlock(&socket_context.filter_mutex);

        }
    }
}

void initialize_socket_context(int raw_socket)
{
    initialize_filters();
    socket_context.raw_socket = raw_socket; 
    socket_context.running = true; 
    socket_context.match = false; 
    pthread_mutex_init(&socket_context.filter_mutex, NULL);
    pthread_cond_init(&socket_context.filter_cond, NULL);
}

bool scan_ssid_channel(int raw_socket, const char* ifname, const char* ssid, int* found_channel)
{
    mac_frame_t* response;
    uint16_t response_len;

    sleep(1);

    while (true)
    {
  
        for (int channel = 1; channel < 14; ++channel)
        {
            printf("Scanning channel: %d\n", channel);
            if (set_channel(raw_socket, ifname, channel) == -1)
            {
                return false;
            }

            if (send_probe_request_to_ssid_with_response(raw_socket, ssid, &response, &response_len, false))
            {
                *found_channel = channel;

                //debug 
                printf("Found channel: %d\n", *found_channel);
                return true;
            }
        }
    }
    
    return false;
}

void initialize_filters()
{
    memset(&socket_context.filters, -1, sizeof(struct filters)); 
    // memset(&socket_context.filters.header.frame_control, 0, sizeof(frame_control_t));
}