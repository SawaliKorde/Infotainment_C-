#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/shm.h>

#define PORT 8080
#define SHM_KEY 1234
#define SHM_SIZE 1024

// Define a structure to hold the client data
struct ClientData {
    int id;
    char name[100];
    char email[100];
    struct ClientData  *prev;
    struct ClientData  *next;
   
};


// Function to create a new node
DNode* createDNode(data) {
	DNode *newNode = (DNode*)malloc(sizeof(DNode));
	if (!newNode) {
		printf("Memory allocation error!\n");
		exit(1);
	}
	newNode->data = data;
	newNode->prev = newNode->next = NULL;
	return newNode;
}
//INSERT NODE
void insertAtEnd(DNode **head, int data) {
DNode *newNode = createDNode(data);
if (*head == NULL) {
*head = newNode;
return;
}
}
//handle client data 
 void *handle_client(void* client_socket){
 	int socket=*(int)* client_socket;	
 	free(client_socket);
 	struct ClientData data;
 	char buffer[1024]={0};
 	
 	while(1){
 		int valread=read(socket,buffer,sizeof(buffer));
 		if(valread<=0){
 			break;
 		}
 		buffer[valread]='\0';
 		if(strcmp(buffer,"add")==0){
 		    read(socket,&data,sizeof(data));
 		    add_data_dlink(data);
 		    char msg="Data add sucessfully.....";
                    send(socket,&msg,strlen(msg),0);
                    printf("\n*----------------------------------------------*\n");
                    printf("ID: %d  Name:%s  Email:%s \n",data.id,data.name,data.email);
                    printf("\n*------------------------------------------------------------------*\n");
                  
 		}
 		else if(strcmp(buffer,"delete")==0){
 			read(socket,&data.name,sizeof(int));
 			delete_data(data.name);
                    	send(socket,&msg,strlen(msg),0);
                    	char msg="Data delete succesfully...";
                    	printf("\n*----------------------------------------------*\n");
                   	printf(" Name:%s \n",data.name);
                   	printf("\n*------------------------------------ ---------*\n");
                      }
               else if(strcmp(buffer,"show")==0){
                    	//show();
                      }
               else if(strcmp(buffer,"search")==0){
 			read(socket,&data.name,sizeof(int));
 			search_data(socket,data.name);
                      }                             	
 	       else if(strcmp="exit")==0{
 	       		close(socket);
 	       		break;
 	       }
 		
 	}
          return NULL;
 }
 
 
 


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation error");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }

    
    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        return -1;
    }

    printf("Server is listening on port %d\n", PORT);

    // Create shared memory segment using shmget
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666); 
    if (shm_id == -1) {
        perror("shmget failed");
        return 1;
    }

   
    void* ptr = shmat(shm_id, NULL, 0);
    if (ptr == (void*)-1) {
        perror("shmat failed");
        return 1;
    }

    memset(ptr, 0, SHM_SIZE);  

    while (1) {
       
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
	printf("Connection Establish\n");
         pthread_t client_thread;
        int* client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = new_socket;
        
         pthread_create(&client_thread, NULL, handle_client, (void*)client_sock_ptr);

        
        close(new_socket);
    }
   }

    return 0;
}
