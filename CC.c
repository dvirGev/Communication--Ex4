#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 

#define SERVER_PORT          3490
#define SERVER_IP_ADDRESS    "127.0.0.1"
#define FILENAME             "file.txt" 
#define FULL_SIZE 1048576
#define BUFF_SIZE 1024

int main() {
   
    char buffer[BUFF_SIZE];
    int size_file;
    FILE *file_pointer;
    socklen_t length;

    int counter_runs = 0;
    int i = 0;
    while(i < 2) {

        int j = 0;
        while(j < 5) {
            // Create clinet socket 
            int client_socket = socket(AF_INET, SOCK_STREAM, 0);
            if(client_socket == -1) {
                fprintf(stderr, "Couldn't create the socket : %s\n", strerror(errno));
                exit(EXIT_FAILURE); // failing exit status.
            }

            int get_sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, &length);
            if( get_sock_opt != 0) {
                perror("getsockopt");
                exit(EXIT_FAILURE); // failing exit status.
            }
            if(i == 0) {
                strcpy(buffer,"cubic");
            } else {
                strcpy(buffer,"reno");
            }
            length = sizeof(buffer);
            int set_sock_opt = setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, length);
            if(set_sock_opt !=0 ) {
                perror("setsockopt");
                exit(EXIT_FAILURE); 
            }
            get_sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, &length);
            if( get_sock_opt != 0) {
                perror("getsockopt");
                exit(EXIT_FAILURE); 
            }
            counter_runs++;
            printf("\nclient (%d) Current CC, type: %s  \n",j, buffer);
            
            struct sockaddr_in server_address;
            memset(&server_address, 0, sizeof(server_address));
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(SERVER_PORT);
            int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &server_address.sin_addr);
            if(rval <= 0) {
                printf("inet_pton() failed");
                return -1;
            }

            // Connect to the server
            int connection = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
            if(connection == -1) {
                fprintf(stderr, "connect() failed with error code:%s\n", strerror(errno));
                exit(EXIT_FAILURE); // failing exit status.
            } 
            else {
                printf("client %d connected to server!\n",j);
            }

            file_pointer = fopen(FILENAME, "r");
            if(file_pointer == NULL) {
                fprintf(stderr, "Failed to open file file.txt : %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            int data_stream;
            int size = 0;
            while( ( data_stream = fread(buffer,1,sizeof(buffer),file_pointer) ) > 0 ) {
                size += send(client_socket, buffer, data_stream, 0);
            }

            if(size == FULL_SIZE) {
                printf("sent all the file file: %d\n",size);
            }else {
                printf("sent just %d out of %d\n",size,FULL_SIZE);
            }
            sleep(1);
            close(client_socket);
            j++;
        }
        i++;
    }
    return 0;
}