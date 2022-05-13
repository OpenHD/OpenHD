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
#include <iostream>

#include "openhd-global-constants.h"

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
    [[nodiscard]] bool verifyNullTerminator()const{
        // check if the string has a null-terminator
        bool nullTerminatorFound=false;
        for(const auto& i : message){
            if(i=='\0'){
                nullTerminatorFound= true;
                break;
            }
        }
        return nullTerminatorFound;
    }
} __attribute__((packed)) localmessage_t;


// these match the mavlink SEVERITY_LEVEL enum, but this code should not depend on
// the mavlink headers
// See https://mavlink.io/en/messages/common.html - MAV_SEVERITY
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

static void print_log_by_level(const STATUS_LEVEL level,std::string message){
    // Each message is logged with a newline at the end, add a new line at the end if non-existing.
    const auto messageN=message.back()=='\n' ? message : (message+"\n");
    if(level==STATUS_LEVEL_INFO || level==STATUS_LEVEL_NOTICE || level==STATUS_LEVEL_DEBUG){
        std::cout<<messageN;
    }else{
        std::cerr<<messageN;
    }
}

/*
 * Messages sent here will end up in the telemetry microservice, where they will be packed up and sent through
 * mavlink for storage and review by qopenhd, the boot screen system, and other software.
 */
inline void ohd_log(STATUS_LEVEL level, const std::string& message) {
    print_log_by_level(level,message);
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
    servaddr.sin_port = htons(OHD_LOCAL_LOG_MESSAGES_UDP_PORT);
    inet_aton("127.0.0.1", (in_addr*)&servaddr.sin_addr.s_addr);

    int n, len; 
      
    sendto(sockfd, &lmessage, sizeof(lmessage), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
}

// Direct implementations for the 3 most common used log types
inline void ohd_log_emergency(const std::string& message){
    ohd_log(STATUS_LEVEL_EMERGENCY,message);
}
inline void ohd_log_info(const std::string& message){
    ohd_log(STATUS_LEVEL_INFO,message);
}
inline void ohd_log_debug(const std::string& message){
    ohd_log(STATUS_LEVEL_DEBUG,message);
}


#endif
