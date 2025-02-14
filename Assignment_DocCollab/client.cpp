#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include<semaphore.h>

using namespace std;

//Creating semaphore variable
sem_t mem;

int sock = 0;
string client_id;
struct sockaddr_in serv_addr;
atomic<bool>take_input;
atomic<bool>read_updates;
#define PORT 8080

void readFromFile(){

  ifstream file("Doc_Collab.txt");
  if(!file){
     cerr << "Cannot open File \n" << endl;
     return;
  }
  string line;
  
  cout << "\033[96m"<< "\n==== Current File Contents ====\n" << "\033[0m";
  while(getline(file,line)){
     cout << "\033[93m" << line << "\033[0m"<<  endl;
  }
  
  cout << "\033[96m" << "\n================================\n"<< "\033[0m";
  //take_input = true;
  //read_updates = false;
}


void sendMsg(){
   take_input = false;
   this_thread::sleep_for(chrono::milliseconds(100));
   string message = "";
    while (true) {
        if(take_input){
           
           cout << "Client: ";
           cout.flush();
           sem_wait(&mem);
           message.clear();
           getline(std::cin, message);
           if(message.empty()){
              sem_post(&mem);
              continue;
           }
            sem_post(&mem);
           
           
           read_updates = true;
           take_input = true;
        }
       
        if (message == "exit") {
            //status = false;
            std::cout << "Client exiting." << std::endl;
            exit(0);
        }
        sem_wait(&mem);
        send(sock, message.c_str(), message.size(), 0);
        sem_post(&mem);
         take_input = true;
         read_updates = true;
        
    }
}

//Receive message from server sent by some client without waiting for send.
void receiveMsg(){
   char buffer[1024] = {0};
   // Continuously listening
   while(read_updates){
       memset(buffer, 0, sizeof(buffer)); // Clear the buffer
       int valread = read(sock, buffer, sizeof(buffer) - 1); // Leave

       if (valread <= 0) break;

       buffer[valread] = '\0'; // Null terminate the received string
       string message(buffer);
       if(message.substr(0, 12) == "FILE_UPDATED"){
           string updater_id = message.substr(13); // Extract the client ID
           cout << "\033[32m" << "File has been updated by -> " << updater_id << "\033[0m" << endl;

           readFromFile();
           cout << "Client: ";
           cout.flush();

           take_input = true;
           read_updates = true;
           continue;
       } else if(message.substr(0, 6) == "Client "){
           client_id = message;
       } else {
           cout << buffer << endl;
           take_input = true;
       }

       if (message == "exit") {
           std::cout << "Connection closed by server." << std::endl;
           break;
       }
   }
}

	

int main() {
    take_input = true;
    sem_init(&mem,0,1);
    //Socket created	
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }
    cout<<"Connected to server"<<endl;
    thread filethread(readFromFile);
     filethread.join();
    take_input = true;
    read_updates = true;
    
    thread messagethread(receiveMsg);
    thread sendthread(sendMsg);
    
    
    messagethread.join();
    sendthread.join();
   
    

    close(sock);
    return 0;
}
