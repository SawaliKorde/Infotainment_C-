#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

int main(){
	//create socket connection
	//write switch cases 
	//send the string input to server

	int socket_fd;
	char buffer[1024] = {0};
	struct sockaddr_in server_address;

	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
	perror("Socket creation failed\n");
	exit(EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0){
	perror("Invalid address");
	exit(EXIT_FAILURE);
	}

	if(connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
	perror("Connetion Failed");
	exit(EXIT_FAILURE);
	}

	printf("Connected to Server\n");
	while(1){
	    printf("\n<----Menu items---->\n");
	    printf("\n Restart \n");
	    printf("\n Stop \n");
	    printf("\n Pause \n");
	    printf("\n Resume \n");
	    printf("\n Exit \n");
	    
	    char message[1024] = {0};
	    fgets(message, sizeof(message), stdin);
	    message[strcspn(message, "\n")] = 0;
	    
	    if(strcmp(message, "Exit")== 0){
	         printf("Closing Connection ...\n");
	         break;
	    }
	    
	    send(socket_fd, message, strlen(message),0);
	    printf("Message Sent\n");
	    
	    int valread = read(socket_fd, buffer, 1024);
	    printf("Server: %s\n", buffer);
	    memset(buffer,0, sizeof(buffer)); 
	}
	close(socket_fd);
	
	return 0;
}
