#include "client_handling.h"
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

struct client_lookup {
    char name[10];             /* key (string is WITHIN the structure) */
    int id;
    UT_hash_handle hh1;         /* makes this structure hashable */
    UT_hash_handle hh2;         //bidrectionally hashable on both
};

struct client_lookup *users = NULL; //global variable 


//pass in the hash table as a pointer here to update in the function 
int is_client_new(int user_id, char *name) {
   

}



int handle_client(int client_fd, int epollfd){

    int numbytes;
    char buf[MAXDATASIZE];
    int welcome;

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

    
        //check for user 
        welcome = is_client_new(client_fd, buf);

        

        //not sure if this all below  should be in the while loop?  
        printf("received: %s\n", buf);
        
        
        //need to fix the string comparison, do after we figure out the received: looping error on disconnect 
        if (strcmp(buf,"/quit\n") == 0){ 
            printf("client disconnected via command /quit ");
        }
    }
}
