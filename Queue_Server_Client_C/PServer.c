#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>  //memory management
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define PORT 8081
#define LOG_FILE "logfile.txt"

/*typedef struct Data {
	int value;
	time_t timestamp;
} Data;

Data *data;

// Define the node structure
typedef struct QNode {
	struct Data *data;
	struct QNode *next;
	} QNode;
	
// Define the queue structure
typedef struct Queue {
	QNode *front;
	QNode *rear;
	} Queue;
	
// Function to create a new node
QNode* createQNode(Data *data) {
	QNode *newNode = (QNode*)malloc(sizeof(QNode));
	if (!newNode) {
		printf("Memory allocation error!\n");
		exit(1);
	}
	newNode->data = data;
	newNode->next = NULL;
	return newNode;
}

void initQueue(Queue *q) {
		q->front = q->rear = NULL;
	}
	
// Enqueue function
void enqueue(Queue *q, Data *data) {
	QNode *newNode = createQNode(data);
	if (q->rear == NULL) {
		q->front = q->rear = newNode;
		return;
	}
	q->rear->next = newNode;
	q->rear = newNode;
}

Data* dequeue(Queue *q) {
	if (q->front == NULL) {
		printf("Queue is empty. Cannot dequeue.\n");
		return NULL;
	}
	QNode *temp = q->front;
	data = temp->data;
	q->front = q->front->next;
	// If front becomes NULL, reset rear to NULL
	if (q->front == NULL)
		q->rear = NULL;
		free(temp);
	return data;
}

void* handleProcess(void* arg){
    int sock = *(int*)arg;
    free(arg);
    //char buffer[1046];
    Data data1;
    while(1){
        int readdata = read(sock, &data1, sizeof(Data));
    	if(readdata <= 0){
    		perror("Failed to read data.");
    	}
    	//buffer[readdata] = '\0';
        //Data data;
        data->value = data1.value;
        data->timestamp = data1.timestamp;
    }

}*/

int main(){
	
     printf("Start....");
     int server_fd, new_socket;
     struct sockaddr_in server_addr;
     int addr_len = sizeof(server_addr);
     //pthread_t readData;
     
     printf("Serverfd socket creating...");
     
     if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
         perror("Creation failed");
         exit(EXIT_FAILURE);
      }
     
     
     printf("Serverfd ...");
     
     server_addr.sin_family = AF_INET;
     server_addr.sin_addr.s_addr = INADDR_ANY;
     server_addr.sin_port = htons(PORT);
     
     if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
     	perror ("Binding Failed.");
     	
        exit(EXIT_FAILURE);
      }
      
      printf("binding...");
      
     if(listen(server_fd, 5)<0){
     	perror("listen failed.");
     	
     	exit(EXIT_FAILURE);
     }
     printf("Server Listening to the PORT %d\n", PORT);
     
     while(1){
     	 new_socket = accept(server_fd, (struct sockaddr *)&server_addr,(socklen_t *)&addr_len);
     	if(new_socket < 0){
     		perror("Accept Failed");
     		return 1;
     	}
     	//pthread_create(&readData, NULL, handleProcess, (void*)&new_socket);
     	close(new_socket);
     }
     
     
     return 0;
}
   
