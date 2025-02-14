#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <map>

#define PORT 8080

using namespace std;

mutex client_mutex;
vector<int> client_sockets;
//vector<thread> threads;
atomic<bool> server_running(true);


void logThreadStatus(const std::string &operation, const std::string &status) {
    
    std::cout << "\nThread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;    
    std::cout.flush();
}

   void handleClient(int client_socket)
   {
   	 
	char buffer[1024] = {0}; //buffer to store client msg
       {	
	//add client socket to vector
	logThreadStatus("Client", "Connected");
	lock_guard<mutex> lock(client_mutex);
	client_sockets.push_back(client_socket);
	
       }
        
	cout<<"New client connected: "<<endl;

      while (true) {
      
      //cout<<"entered while loop"<<endl;
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading
        int valread = read(client_socket, buffer, sizeof(buffer) - 1); // Leave space for null terminator.
        if (valread <= 0){
	   cout<<"Client disconnected"<<endl;	
	   break;
         }
         
        buffer[valread] = '\0'; // Null terminate the received string
        cout << "Client: " << buffer << endl;
        cout<<"Server Starting"<<endl;

        if (string(buffer) == "exit") {
            cout << "Connection closed by client." << endl;
            break;
        }

        string message;
        cout << "Server: ";
        getline(cin, message);
        send(client_socket, message.c_str(), message.size(), 0);

        if (message == "exit") {
            server_running = false;
            cout << "Server exiting." << std::endl;
            //break;
           exit(0);
        }
       }
        close(client_socket);
        
        
  }    
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;// structure to hold server address details
    int addrlen = sizeof(address);// length of add struc
    vector<thread>client_threads;
    
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
    
	
   while(true){ 
   logThreadStatus("Socket", "Created");
    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen); // waits for an accepts conne from client
      if(new_socket<0){
        cout<<"Failed to connect client"<<endl;
         break;
     }
    
    //client_cnt++;
    //cout<<"new client"<<client_cnt<<"connected"<<endl;
    
    client_threads.emplace_back(handleClient, new_socket);    
    } 
    logThreadStatus("Thread", "Closing");  
     for(auto& thread : client_threads){
      if(thread.joinable()){
      thread.join();
    }
  }

    close(new_socket);
    close(server_fd);
    return 0;
}
