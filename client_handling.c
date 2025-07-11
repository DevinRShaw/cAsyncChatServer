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
    char name[MAXDATASIZE];             /* key (string is WITHIN the structure) */
    int id;
    UT_hash_handle hh1;         /* makes this structure hashable */
    UT_hash_handle hh2;         //bidrectionally hashable on both
};

struct client_lookup *users_id = NULL; //global variable <client, name>
struct client_lookup *users_name = NULL; //global variable <name, client> 


//pass in the hash table as a pointer here to update in the function 
int is_client_new(int user_id) {
   
    struct client_lookup *s;

    //is the client on the server already
    HASH_FIND(hh1, users_id, &user_id, sizeof(user_id), s);
    if (s == NULL) {
        return 1; //they are not on server so you return 0;
    }

    //will always be 0 until you do the add user 

    return 0; //if present on the server then just return 1 to say yes 
}

//make sure the string look up is correct 

int is_username_unique(char* name){
    //change to the type of string in the struct 
    char copy_name[MAXDATASIZE];
    strcpy(copy_name, name);


    struct client_lookup *s;

    HASH_FIND(hh2, users_name, &copy_name, strlen(copy_name), s);
    if (s == NULL) {
        return 1; //they are not unique so you return 0;
    }

    return 0; //if not unique then just return 1 to say yes 
}

//the username part of this is not working
void add_user(int user_id, char* name){
    //change to the type of string in the struct 
    char copy_name[MAXDATASIZE];
    strcpy(copy_name, name);


    struct client_lookup *s = malloc(sizeof *s);
    s->id = user_id;
    strcpy(s->name, copy_name);

    HASH_ADD(hh1, users_id, id, sizeof(int), s);
    HASH_ADD(hh2, users_name, name, strlen(s->name), s);
}



//---------------------------------------------------------------------------------


int handle_client(int client_fd, int epollfd){

    int numbytes;
    char buf[MAXDATASIZE];
    int welcome;
    int unique_username;

    while (1) {

        memset(buf, 0, sizeof(buf));
        //printf("in the recv loop: ");
        numbytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (numbytes == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data to read, breaks the loop for us 
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

        unique_username = is_username_unique(buf);
        welcome = is_client_new(client_fd);

        if (welcome == 1){ //if not on server yet 
            if (unique_username == 1){ //if username is unique 
                add_user(client_fd, buf);
            }
            else {
                //this will not work without having a proper listening client, can be done with send and listen thread client side 
                send(client_fd, "that username is taken", sizeof("that username is taken"), 0);
            }
        }


        //the buffer needs newline to print to terminal, line based buffer 
        printf("received: %s\nnew client: %d\nnew username: %d\n", buf, welcome, unique_username);

        
        //need to fix the string comparison, do after we figure out the received: looping error on disconnect 
        if (strcmp(buf,"/quit\n") == 0){ 
            printf("client disconnected via command /quit\n");
        }
    }
}
