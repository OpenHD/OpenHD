#ifndef OPENHD_STATUS_H
#define OPENHD_STATUS_H


#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include <string>

/**
 * This provides convenient methods to send log messages from any service running on the air or ground unit to a final output deice,
 * for example QOpenHD.
 * As an example, this makes it possible to view the logs from the air unit on QOpenHD without connecting a display to the air unit.
 * The general way this works is simple:
 * The log messages is sent to a specific udp port on localhost and then picked up by the telemetry service,
 * which converts it to mavlink and forwards it accordingly.
 */

typedef struct {
    uint8_t level;
    uint8_t message[50];
} __attribute__((packed)) localmessage_t;


// these match the mavlink SEVERITY_LEVEL enum, but this code should not depend on
// the mavlink headers
typedef enum STATUS_LEVEL {
    STATUS_LEVEL_EMERGENCY = 0,
    STATUS_LEVEL_ALERT,
    STATUS_LEVEL_CRITICAL,
    STATUS_LEVEL_ERROR,
    STATUS_LEVEL_WARNING,
    STATUS_LEVEL_INFO,
    STATUS_LEVEL_NOTICE,
    STATUS_LEVEL_DEBUG
} STATUS_LEVEL;

/*
 * Messages sent here will end up in the status microservice, where they will be packed up and sent through
 * the microservice channel for storage and review by qopenhd, the boot screen system, and other software.
 *
 */
inline void status_message(STATUS_LEVEL level, std::string message) {

    std::cerr << message << std::endl;

    localmessage_t lmessage;
    lmessage.level = static_cast<uint8_t>(level);
    strncpy((char*)lmessage.message, message.c_str(), 50);
    if (lmessage.message[49] != '\0') {
        lmessage.message[49] = '\0';
    }

    int sockfd; 

    struct sockaddr_in servaddr; 
  
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("Socket create failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(50000); 
    inet_aton("127.0.0.1", (in_addr*)&servaddr.sin_addr.s_addr);

    int n, len; 
      
    sendto(sockfd, &lmessage, sizeof(lmessage), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
}


#endif
