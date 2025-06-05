#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    //getaddrinfo = sets up the info we need to set up socket, can be used for other address too, ai_passive for ours 


    /*

    int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                    const char *service,  // e.g. "http" or port number
                    const struct addrinfo *hints,
                    struct addrinfo **res);
                    
            
    struct addrinfo {
                int              ai_flags;
                int              ai_family;
                int              ai_socktype;
                int              ai_protocol;
                socklen_t        ai_addrlen;
                struct sockaddr *ai_addr;
                char            *ai_canonname;
                struct addrinfo *ai_next;
            };



    */

    //this is a struct we use to as hint to pass into getaddrinfo setup 

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // servinfo now points to a linked list of 1 or more struct addrinfos

    // ... do everything until you don't need servinfo anymore ....

    //freeaddrinfo(servinfo); // free the linked-list


    //set the socket up to that now 

    int sockfd; // this is the socket file descriptor, values already handles in getaddringo

    //could have hardcoded this, but above handles set up later too, ai passive key 
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);


    bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);


 
    //char yes='1'; // Solaris people use this
    int yes=1;
    // lose the pesky "Address already in use" error message
    if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    } 


    //we set up the addr info, set that to a socket, binded to a port, now we connect 
    //this is for connecitng to a remote host not listening so we will save that for later 
        //doesn't requrie bind since we connecting to their port 
    //connect( sockfd, servinfo->ai_addr, servinfo->ai_addrlen); 

    int backlog = 10; //number of connections that can wait in queue 

    listen(sockfd, backlog);

    if (listen(sockfd, backlog) == -1) {
        perror("listen error");
        exit(1);
    } 



    // now accept an incoming connection:
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);



    char *msg = "Beej was here!";
    int len, bytes_sent;

    
    len = strlen(msg);
    bytes_sent = send(sockfd, msg, len, 0);


    return 0;
}