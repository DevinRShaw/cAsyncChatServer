#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "uthash/src/uthash.h"

#define MAXDATASIZE 100000 // max number of bytes we can get at once

int handle_client(int client_fd, int epollfd ){

    int numbytes;
    char buf[MAXDATASIZE];

    while (1) {

        memset(buf, 0, sizeof(buf));
        //printf("in the recv loop: ");
        numbytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (numbytes == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data to read
                return -1;
                break;
            } 
            
            else {
                perror("recv");
                close(client_fd);
                return -1;
                break;
            }
        }

        //to establish that a client has been disconnected 
        else if (numbytes == 0){
            printf("connection closed\n");
            close(client_fd);
            epoll_ctl(epollfd, EPOLL_CTL_DEL, client_fd, NULL); // Remove from epoll
            return 0;
            break;
        }
    
    
        //not sure if this all below  should be in the while loop?  
        printf("received: %s\n", buf);
        
        
        //need to fix the string comparison, do after we figure out the received: looping error on disconnect 
        if (strcmp(buf,"/quit\n") == 0){ 
            printf("client disconnected via command /quit ");
        }
    }
}


int main() {

//getaddrinfo = sets up the info we need to set up socket, can be used for other address too, ai_passive for ours 

    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results, a linked list of 1 or more struct addrinfos, many for different configs 


    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }



//setting the socket up to that now, previous work ^ is useful here, handles a lot for us 

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);

    freeaddrinfo(servinfo); // free the linked-list after done using servinfo to set up the server 

    //lose the pesky "Address already in use" error message after restart 
        int yes=1;
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
            perror("setsockopt");
            exit(1);
        } 


//begin to listen on the socket

    int backlog = 10; //number of connections that can wait in queue, not sure what should be for each use case 

    if (listen(sockfd, backlog) == -1) {
        perror("listen error");
        exit(1);
    } 


//end of the beej tutorial socket setup, beginning of epoll man page example usage 


    #define MAX_EVENTS 10
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd;

    listen_sock = sockfd;

    fcntl(listen_sock, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size = sizeof their_addr;


    epollfd = epoll_create1(0);


    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {

                printf("server: waiting for connections...\n");

                conn_sock = accept(listen_sock, (struct sockaddr *) &their_addr, &sin_size);

                if (conn_sock == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                //this was originally setnonblocking(sockfd)
                fcntl(conn_sock, F_SETFL, O_NONBLOCK);
                
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                            &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }

                printf("server: client connection configured\n");

                send(conn_sock, "who are you?", 11, 0);



            } else {
                handle_client(events[n].data.fd, epollfd);
            }

        }
    }
    
    return 0;
}


