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
#include<algorithm>
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
vector<int> clients_socket; //used for client sockets connections
string received_message; //Client message stored in string message

atomic<bool> is_message_written;
atomic<bool> if_message_received;

void logThreadStatus(const std::string &operation, const std::string &status) {
    std::cout << "\nThread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;    
    std::cout.flush();
}

//Handle Client
void handleClient(int client_socket, thread& write_mem_thread, thread& read_mem_thread){
   char buffer[1024] = {0};
   sem_wait(&mem);
   clients_socket.push_back(client_socket);
   string client_id = "Client " + to_string(clients_socket.size());
   send(client_socket, client_id.c_str(), client_id.length(), 0);
   sem_post(&mem);
   
   while(true){
     memset(buffer, 0, sizeof(buffer)); 
     int valread = read(client_socket, buffer, sizeof(buffer) - 1);
     if (valread <= 0){
        cout<<"Client disconnected"<<endl;
        
        clients_socket.erase(find(clients_socket.begin(),clients_socket.end(),client_socket));
        close(client_socket);
        break;
     }
     string received_message(buffer);
     {
        sem_wait(&mem);
        string message_with_id = client_id + ":" + received_message;
        clientMessages.push_back(message_with_id);
        if_message_received = true;
        cout << "Message received from client is: \n" << received_message << endl;
        sem_post(&mem);
     }
   }
}

//Writing the data to SM
void writeDataToMemory() {
   size_t total_memory_used = 0;
   
   while (true) {
        if (if_message_received) {
            cout << "\n-----------------------------------" << endl;
            cout << "Current memory usage : : "<< total_memory_used << "/" << shm_size << "bytes" << endl;
            cout << "\n-----------------------------------" << endl;
            std::cout << "\nThread [" << std::this_thread::get_id() << "] - "<< std::endl;
          
            if(!clientMessages.empty()){
                sem_wait(&mem);
                string received_message = clientMessages.front();
                clientMessages.erase(clientMessages.begin());
                sem_post(&mem);
        
                if(!received_message.empty()){
                    size_t message_size = received_message.length() + 1;
                    if(total_memory_used + message_size <= shm_size){
                        sem_wait(&mem);
                        memcpy(ptr1, received_message.c_str(), received_message.length() + 1); //Writing to SM
                        total_memory_used += message_size;
                        is_message_written = true;
                        sem_post(&mem);
                        cout << "Message written to shared memory\n" << received_message << endl;
                    }else{
                        cout <<"Limit reached --->  Resetting the size"<<endl;
                        sem_wait(&mem);
                        memset(ptr1,0,shm_size);
                        total_memory_used = 0;
                        sem_post(&mem);
                    }
                }
            }
            if_message_received = false;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void readSharedMemory(){
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cerr << "Shared memory opening failed for "<< "\n";
        return;
    }

    ofstream file("Doc_Collab.txt", ios::out | ios::app);
    if (!file) {
        cerr << "Cannot open the file to write.\n";
        return;
    }

    while (true) {
        if (is_message_written) {
            sem_wait(&mem);
            void* ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
            if (ptr == MAP_FAILED) {
                cerr << "Mapping shared memory failed for " << "\n";
                sem_post(&mem);
                return;
            }
            string received_message(static_cast<char*>(ptr));
            munmap(ptr, shm_size);
            sem_post(&mem);

            file << received_message << endl;
            cout << " New Message written in Document Collaborator:\n"<< received_message << endl;

            // Extract the client ID from the message
            size_t colon_pos = received_message.find(":");
            string client_id = received_message.substr(0, colon_pos);

            // Broadcast to all clients
            string update_clients = "FILE_UPDATED " + client_id;
            sem_wait(&mem);
            for(int i = 0; i < clients_socket.size(); i++){
                send(clients_socket[i], update_clients.c_str(),update_clients.length(), 0);
            }
            sem_post(&mem);
            is_message_written = false;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
}

int main(){
    sem_init(&mem,0,1); //semaphore
    
    int server_fd, new_socket, client_socket; //socket
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    vector<thread> client_threads;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }
    
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666); //shared mewmory
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

    cout << "Server started! \n"<< endl;
    
    // Start the shared memory write and read threads once
    thread write_mem_thread(writeDataToMemory);
    thread read_mem_thread(readSharedMemory);
    
    while(true){ 
        logThreadStatus("Socket", "Created");
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        cout<<"New socket: "<< new_socket<<endl;
        if(new_socket<0){
            cout<<"Failed to connect client"<<endl;
            break;
        }
        
        client_threads.emplace_back(handleClient, new_socket, ref(write_mem_thread), ref(read_mem_thread));
    } 
    //Clean up code
    logThreadStatus("Thread", "Closing");  
    for(auto& thread : client_threads){
        if(thread.joinable()){
            thread.join();
        }
    }

    if(write_mem_thread.joinable()) write_mem_thread.join();
    if(read_mem_thread.joinable()) read_mem_thread.join();

    sem_destroy(&mem);
    munmap(ptr1, shm_size);
    shm_unlink(shm_name);
    
    close(new_socket);
    close(server_fd);
    return 0;
}
