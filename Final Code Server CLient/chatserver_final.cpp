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
#include<regex>
#include<iomanip>
#include<chrono>
#include<sstream>

#define PORT 8080

using namespace std;

mutex client_mutex;
//vector<int> client_sockets;
//vector<thread> threads;
atomic<bool> server_running(true);
int countid = 0;
string message;
atomic<bool>status;

map<int, bool> client_status;
map<int, int> client_map;

void logThreadStatus(const std::string &operation, const std::string &status) {
    
    std::cout << "\nThread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;    
    std::cout.flush();
}

	
   void handleClient(int client_socket, string message)
   {
   	 int client_id;
	char buffer[1024] = {0}; //buffer to store client msg
       {
	//add client socket to vector
	logThreadStatus("Client", "Connected");
	lock_guard<mutex> lock(client_mutex);
	client_id = countid;
	client_map[countid] = client_socket;
	client_status[client_id] = true;
	++countid;
	//client_sockets[countid] = client_socket;
	
	
	if(client_map.empty()){
	  cerr<<"Client map is empty."<<endl;
	} else{  
	  for (const auto& [id,socket]: client_map){
	  cout<<"ID: "<< id  << "Client Socket"<< socket <<endl;
	  cout<<"New client connected: "<<endl;
	}
	}
       }


      while (true) {
      
      //cout<<"entered while loop"<<endl;
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading
        int valread = read(client_socket, buffer, sizeof(buffer) - 1); // Leave space for null terminator.
        if (valread <= 0){
           client_status[client_id] = false;
           this_thread::sleep_for(chrono::seconds(8));
	   cout<<"Client disconnected"<<endl;
	   client_map.erase(client_id);
	   break;
         }
         
        buffer[valread] = '\0'; // Null terminate the received string
        cout << "Give message in this format 'client_id: message' " << endl;
        cout<<"Server Starting"<<endl;
	string message = string(buffer);
	if (message == "/list") {
	    cout << "Connected Clients:\n";

	    stringstream response; // Use stringstream for simpler concatenation

	    lock_guard<mutex> lock(client_mutex);
	    for (const auto &[id, socket] : client_map) {
		if (client_status[id]) {
		    response << "ID: " << id << ", Socket: " << socket << "\n"; // Add client info
		}
	    }

	    send(client_socket, response.str().c_str(), response.str().size(), 0); // Send the response to the client
	    continue;
	}
        if (string(buffer) == "exit") {
            /*for(auto clnt = client_map){
	       if(clnt.second == client_socket){
 		  client_map.erase(clnt.first);
		}
	    }*/
            cout << "Connection closed by client." << endl;
            break;
        }

        
	regex messageRegex("^\\d+:.+$");
        if (!regex_match(message, messageRegex)) {
           cout<< "Invalid Message Format --> MESSAGE FORMAT ---> CLIENT_ID:MESSAGE"<<endl; --> named pipes
            continue;
        }
        
        string cid = message.substr(0, message.find(":"));
        try{
            int cid1 =  stoi(cid); // DP id should be saved
       
	 	for (const auto& client: client_map)
	 	{
	 	    cout<<"Client: "<< client.first<<" DP ID: "<<client.second<<endl;
	 	    //cout<<"TYPE"<<typeid(client.second).name()<<endl;
	 	    if(cid1 == client.second){
	 	            
		    string sendmessage = message.substr(message.find(":") + 1); // should save message of client
		    cout<<"Send message: "<< sendmessage <<endl;
		    send(client.second, sendmessage.c_str(), sendmessage.size(), 0); // To send to client
		    
		    //send(client_socket, message.c_str(), message.size(), 0);
		 }
        }
	}catch(const exception& e){
	   	cout<< "Error"<< e.what() <<endl;
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
  void display(){
	while(true){
		
		cout << "\n+---------+----------------+----------------+---------------------+--------------+\n";
		std::cout << "| " << std::setw(15) << left << "Socket DP ID"
		  << "| " << std::setw(15) << left<< "IP Address"
		  << "| " << std::setw(15)<< left << "Thread ID"
		  << "| " << std::setw(10)<< left << "Client Status" << " |" <<std::endl;
		cout << "+---------+----------------+----------------+---------------------+--------------+\n";
		
		   for (const auto& client : client_map) {
		        string status_emoji = client_status[client.first] ? "\033[32m☺\033[0m" : "\033[31m☹\033[0m";
		        cout << "| " << setw(15) << left << client.second 
		             << "| " << setw(15) << left << "127.0.0.1"
		             << "| " << setw(15) << right << this_thread::get_id()
		             << "| " << setw(10) << right << status_emoji << " |" << endl;
		        cout << "+---------+----------------+----------------+---------------------+--------------+\n";
            }
		this_thread::sleep_for(chrono::seconds(2));
		system("clear");
		}
}

int main() {

    thread displaythread(display);	//Thread for displaying panel
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
    cout<<"New socket: "<< new_socket<<endl;
      if(new_socket<0){
        cout<<"Failed to connect client"<<endl;
         break;
     }
    
    //client_cnt++;
    //cout<<"new client"<<client_cnt<<"connected"<<endl;
    
    client_threads.emplace_back([=](){handleClient(new_socket,message);});
    
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
