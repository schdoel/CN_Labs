#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "header.h"

#define PORT 45525
#define BUF_SIZE 1024

int main(int argc , char *argv[]){

    //Create TCP socket//

    //Set up server's address.//

    //Bind socket to the address.//  

    //Listening the socket.//

    //Accept the connect request.//

    //Send message to client.//


    ////////////////////////////////////////////////////////////
    //                   TASK 1(Server)                       //
    ////////////////////////////////////////////////////////////
    // TODO: Create a TCP socket bind to port 45525.          //
    // TODO: Listen the TCP socket.                           //
    // TODO: Accept the connect and get the Client socket     //
    //       file descriptor.                                 //
    // TODO: Send 1 message "Hi, I'm server {Your_student_ID}"//
    //       to client.                                       //
    // Then go finish the client.c TASK 1                     //
    ////////////////////////////////////////////////////////////
    int socket_fd, client_fd;
    struct sockaddr_in server_address, client_addr;
    socklen_t addr_size;
    char buffer[BUF_SIZE];

    //Create TCP socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        printf("[-]Socket failed");
        exit(1);
    }
    printf("[+]TCP Socket created\n");
    memset(&server_address, '\0', sizeof(server_address));

    //Set up server's address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);

    // Bind socket to the address
    int n = bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)); 
    if(n < 0) {
        printf("[-]Bind failed");
        exit(1);
    }
    printf("[+]Done binding to Port number %d.\n", PORT);

    // Listening the socket
    if (listen(socket_fd, 5) < 0) {
        printf("[-]Listen failed");
        return -1;
    }
    printf("[+]Listening for incoming connections...\n");

    // Accept the connect request
    client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_size);
    if (client_fd < 0) {
        printf("[-]Accept failed");
        return -1;
    }
    strcpy(buffer, "Hi, I'm server 109006205\n");
    send(client_fd, buffer, strlen(buffer), 0);
    
    // printf("[+]Closing connection\n");
    
    ////////////////////////////////////////////////////////////
    //                   TASK 2,3(Server)                     //
    ////////////////////////////////////////////////////////////
    // TODO: Pass the client socket fd into serverfuntion()   //
    //                                                        //
    // Example:                                               //
    //           serverfunction(client_fd);                   //
    //                                                        //
    // Then go finish the client.c TASK2,3                    //
    ////////////////////////////////////////////////////////////

    // Pass the client socket fd into serverfunction()
    serverfunction(client_fd);

    return 0;
}
