    #ifndef CLIENT_HANDLING_H
    #define CLIENT_HANDLING_H

    int handle_client(int client_fd, int epollfd);
    int is_client_new(int user_id, char *name); 
        //1 if yes
        //0 if no, then adds that client and names to the hash 

    #endif