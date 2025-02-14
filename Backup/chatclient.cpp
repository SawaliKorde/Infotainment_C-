#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <atomic>

using namespace std;
mutex receive_mutex;
int sock = 0;
struct sockaddr_in serv_addr;

#define PORT 8080

//Receive message from server sent by some client without waiting for send.
void receiveMsg(){
   char buffer[1024] = {0};
   //Continuosly listening
   while(true){
   memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        int valread = read(sock, buffer, sizeof(buffer) - 1); // Leave space for null terminator
        if (valread <= 0) break;

        buffer[valread] = '\0'; // Null terminate the received string
        std::cout << buffer << std::endl;

        if (std::string(buffer) == "exit") {
            std::cout << "Connection closed by server." << std::endl;
            break;
        	}
        }
   
}

void sendMsg(){
    while (true) {
        std::string message;
        std::cout << "Client: ";
        std::getline(std::cin, message);
        

        if (message == "exit") {
            //status = false;
            std::cout << "Client exiting." << std::endl;
            exit(0);
        }
        send(sock, message.c_str(), message.size(), 0);
        
    }
}

int main() {
    //char buffer[1024] = {0};
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
    thread messagethread(receiveMsg);
    sendMsg();
    
    messagethread.join();

    close(sock);
    return 0;
}
