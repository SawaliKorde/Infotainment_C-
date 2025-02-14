#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <mutex>
#include <atomic>
#include <map>
#include<iomanip>
#include<chrono>
#include<fstream>


#define PORT_A 8080
#define PORT_B 8081

using namespace std;

mutex client_mutex;
atomic<bool> server_running(true);
atomic<bool>serverConnection(false);//I'm still alive

map<int, bool> client_status;
map<int, int> client_map;

int countid = 0;
string message;
int sock;


void connectServer()
{

	this_thread::sleep_for(chrono::seconds(8));

	sock = 0;
	struct sockaddr_in serv1_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return;
    }
    

    serv1_addr.sin_family = AF_INET;
    serv1_addr.sin_port = htons(PORT_B);

    
    if (inet_pton(AF_INET, "127.0.0.1", &serv1_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return;
    }


    while(true){
    if (connect(sock, (struct sockaddr*)&serv1_addr, sizeof(serv1_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        serverConnection = false;
        this_thread::sleep_for(chrono::seconds(5));
    } else{
    	 cout << "Server 1 connected to Server 2..." << endl;
    	serverConnection = true;
    	break;
     }  
   }
 }

    void logThreadStatus(const std::string &operation, const std::string &status) {
    
	    std::cout << "\nThread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;    
	    std::cout.flush();
    }

	
   void handleClient(int client_socket, string message)
   {
        ofstream outfile("server1.txt", ios::app);
        if(!outfile){
	   cerr << "Can't open file" << endl;
	   return;
	}
   	int client_id;
	char buffer[1024] = {0}; //buffer to store client msg
	//char buffer2[1024] = {0};
       {
	//add client socket to vector
	logThreadStatus("Server", "Connected");
	lock_guard<mutex> lock(client_mutex);
	client_id = countid;
	client_map[countid] = client_socket;
	client_status[client_id] = true;
	++countid;
	
	if(client_map.empty()){
	  cerr<<"Client map is empty."<<endl;
	} else{  
	  for (const auto& [id,socket]: client_map){
	  cout<< "New server connected: " << "ID: "<< id  << " Client Socket: "<< socket <<endl;
	 }
	}
      }

      logThreadStatus("Server", "Started");
      		
      while (true) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading
        int valread = read(client_socket, buffer, sizeof(buffer) - 1); // Leave space for null terminator.
        if (valread <= 0){
           client_status[client_id] = false;
           this_thread::sleep_for(chrono::seconds(5));
	   cout<<"Server disconnected"<<endl;
	   client_map.erase(client_id);
	   break;
         }
         
        buffer[valread] = '\0'; // Null terminate the received string
        cout << "Recieved From Server 2: " << buffer << std::endl;
        outfile << "Messages from server2: " << buffer << endl; 
        if (string(buffer) == "exit") {
            cout << "Connection closed by client." << endl;
            break;
        }
        
        if (message=="exit") {
            //server_running = false;
            cout << "Server exiting." << std::endl;
            //break;
           exit(0);
        }
        
       }
        close(client_socket);
        
  }    
  //Receive message from server sent by some client without waiting for send.
/*void receiveMsg(){
   char buffer[1024] = {0};
   //Continuosly listening
   while(true){
   memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        int valread = read(sock, buffer, sizeof(buffer) - 1); // Leave space for null terminator
        if (valread <= 0){
            std::cerr << "Server disconnected." << std::endl;
            serverConnection = false;
            close(sock);
            break;
	}

        buffer[valread] = '\0'; // Null terminate the received string
        std::cout << "Received from Server 2: "<< buffer << endl;
        
        }
   
}*/

void sendMsg(){
     while (true) {
        if (!serverConnection) {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before retrying
            continue;
        }

        string message = "I am also here";
        int sent = send(sock, message.c_str(), message.size(), 0);
        if (sent <= 0) {
            std::cerr << "Failed to send message. Server might be disconnected." << std::endl;
            serverConnection = false;
            close(sock); // Close the socket
            std::this_thread::sleep_for(std::chrono::seconds(5));
            connectServer();
            continue;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Send every 5 seconds
    }
}
  

int main() {
	//thread receive(receiveMsg);
	thread send(sendMsg);
    	thread serverC_thread(connectServer);
    
    int server_fd, new_socket;
    struct sockaddr_in address;// structure to hold server address details
    int addrlen = sizeof(address);// length of add struc
    vector<thread>client_threads;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { //creates new socket and 
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY;// accepts connection from any...
    address.sin_port = htons(PORT_A); //setting to port 8080

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { //associate socket with specific IP and port
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) { // marks the socket as passive accept 3 connections requests
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }
    
   logThreadStatus("Socket", "Created");
   while(true){ 
    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen); // waits for an accepts conne from client
    cout<<"New socket: "<< new_socket<<endl;
      if(new_socket<0){
        cout<<"Failed to connect client"<<endl;
         break;
     }
    client_threads.emplace_back([=](){handleClient(new_socket,message);});
    
    }
    
    //logThreadStatus("Thread", "Closing");  
     for(auto& thread : client_threads){
      if(thread.joinable()){
      thread.join();
    }
  }
  
	serverC_thread.join();
	//receive.join();
    	send.join();
	

    //close(new_socket);
    close(server_fd);
    return 0;
}
