#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include<semaphore.h>
#include<vector>
#include<chrono>
#include<sstream>
#include<string>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include<fstream>

#define PORT 8080

using namespace std;

//Creating shared memory
const char* shm_name = "/my_shared_memory";
const int shm_size = 4096;

//Creating semaphore variable
sem_t mem;
void* ptr1;
//Defining vector for client messagess

vector<string> clientMessages;
string received_message; //Client message stored in string message


atomic<bool> is_message_written;
atomic<bool> if_message_received;
void logThreadStatus(const std::string &operation, const std::string &status) {
    
    std::cout << "\nThread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;    
    std::cout.flush();
}

//Handle Client
void handleClient(int client_socket){
   char buffer[1024] = {0};
   while(true){
     memset(buffer, 0, sizeof(buffer)); 
     int valread = read(client_socket, buffer, sizeof(buffer) - 1); // Leave space for null terminator.
        if (valread <= 0){
           
           //this_thread::sleep_for(chrono::seconds(8));
	   cout<<"Client disconnected"<<endl;
	   close(client_socket);
	   break;
         }
         string received_message(buffer);
         {
          sem_wait(&mem);
          clientMessages.push_back(received_message);
          if_message_received = true;
          cout << "Message received from client is: " << received_message << endl;
          sem_post(&mem);
         }
         
         }
}

//Writing the data to SM
void writeDataToMemory() {
   cout<<"funcion is called and working"<<endl;
   size_t total_memory_used = 0;
   
   	
    while (if_message_received) {
         cout << "\n-----------------------------------" << endl;
         cout << "Current memory usage : : "<< total_memory_used << "/" << shm_size << "bytes" << endl;
         cout << "\n-----------------------------------" << endl;
         std::cout << "\nThread [" << std::this_thread::get_id() << "] - "<< std::endl;
          
        //sem_wait(&mem);
        if(!clientMessages.empty()){
                sem_wait(&mem);
        	string received_message = clientMessages.front();
                clientMessages.erase(clientMessages.begin());
                sem_post(&mem);
        
        }
        if(!received_message.empty()){
           
            size_t message_size = received_message.length() + 1;
            if(total_memory_used + message_size <= shm_size){
                sem_wait(&mem);
               memcpy(ptr1, received_message.c_str(), received_message.length() + 1);
               total_memory_used += message_size;
               is_message_written = true;
                sem_post(&mem);
               cout << "Message written to shared memory" << received_message << endl;
            }else{
              cout <<"Limit reached --->  Resetting the size"<<endl;
              sem_wait(&mem);
              memset(ptr1,0,shm_size); // put this change
              total_memory_used = 0;
              sem_post(&mem);
             }
        this_thread::sleep_for(chrono::seconds(3));
    }
}
}

void readSharedMemory(){
  int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cerr << "Shared memory opening failed for "<< "\n";
        return;
    }
    //File creation
   
    ofstream file("Doc_Collab.txt", ios::out | ios::app);
    if (!file) {
        cerr << "Cannot open the file to write.\n";
        return;
    }
    
    while (true) {
        sem_wait(&mem);
        void* ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            cerr << "Mapping shared memory failed for " << "\n";
            return;
        }
        string received_message(static_cast<char*>(ptr));
        munmap(ptr, shm_size);
        sem_post(&mem);

        if (is_message_written) {
            file << received_message << endl; //writing to file
            cout << " New Message writen in Document Collaborator:\n" << received_message << "\n";
            is_message_written = false;
            
        }

        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main(){
	//string fileName;
	
	sem_init(&mem,0,1); //semaphore initialization
	
	int server_fd, new_socket, client_socket;
	struct sockaddr_in address;// structure to hold server address details
	int addrlen = sizeof(address);// length of add struc
	
	vector<thread>client_threads; //threads for handling the messages
	
	//Starting the threads
	//thread handle_client_thread(handleClient,client_socket);
	 
	
	 //int client_cnt = 1;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { //creates new socket and 
	std::cerr << "Socket creation error" << std::endl;
	return -1;
	}
	
	address.sin_family = AF_INET; 
        address.sin_addr.s_addr = INADDR_ANY;// accepts connection from any...
        address.sin_port = htons(PORT); //setting to port 8080

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { //associate socket with specific IP and port
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) { // marks the socket as passive accept 3 connections requests
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }
    
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "Shared memory creation failed\n";
        return 1;
    }

    
    if (ftruncate(shm_fd, shm_size) == -1) {
        cerr << "Failed to set shared memory size\n";
        return 1;
    }

    
    ptr1 = mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr1 == MAP_FAILED) {
        cerr << "Mapping shared memory failed\n";
        return 1;
    }

    cout << "Server started! Writing data to shared memory...\n";
    
	
   while(true){ 
   logThreadStatus("Socket", "Created");
    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen); // waits for an accepts conne from client
    cout<<"New socket: "<< new_socket<<endl;
      if(new_socket<0){
        cout<<"Failed to connect client"<<endl;
         break;
     }
    
    //client_cnt++;
    //cout<<"new client"<<client_cnt<<"connected"<<endl;
    
    client_threads.emplace_back([=](){handleClient(new_socket);});
    thread write_mem_thread(writeDataToMemory);
    thread read_mem_thread(readSharedMemory);
    
     read_mem_thread.join();
     write_mem_thread.join();
    } 
    //CLean up 
    logThreadStatus("Thread", "Closing");  
     for(auto& thread : client_threads){
      if(thread.joinable()){
      thread.join();
      
    }
  
    
  }
  //handle_client_thread.join();

    sem_destroy(&mem);
   
   munmap(ptr1, shm_size);
   shm_unlink(shm_name);
	
    close(new_socket);
    
    close(server_fd);
    return 0;
    
}
