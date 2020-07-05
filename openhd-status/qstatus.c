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


typedef struct {
    uint8_t level;
    uint8_t message[50];
} __attribute__((packed)) localmessage_t;



int main(int argc, char **argv) {
    if (argc < 3) {
        exit(1);
    }

    int level = atoi(argv[2]);

    localmessage_t message;
    message.level = level;
    strncpy(message.message, argv[1], 50);
    if (message.message[49] != '\0') {
        message.message[49] = '\0';
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
    inet_aton("127.0.0.1", &servaddr.sin_addr.s_addr);

    int n, len; 
      
    sendto(sockfd, &message, sizeof(message), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 

    
    return 0;
}
