#ifndef SETTINGS
#define SETTINGS

#define SPOOF_MAC_ADDRESS  0x010203040506ULL  //mac address stored as little endian
#define MAX_FRAME_SIZE 2048
#define SSID "FASTWEB-D"
#define MAC_LEN 6
#define MAC_FRAME_SIZE 2048
#define FRAME_QUEUE_SIZE 2048
#define KiB(n) ((uint64_t)(n) << 10)  //1024
#define MiB(n) ((uint64_t)(n) << 20)  //1048576 
#define GiB(n) ((uint64_t)(n) << 30) //1073741824

#endif