#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#define SERVER1_PORT 8080

void server1_thread(int server1_socket) {
    char buffer[1024] = {0};
    while (true) {
        // Receive from Server 2
        int valread = read(server1_socket, buffer, 1024);
        if (valread > 0) {
            std::cout << "Server 1 received: " << buffer << std::endl;
        }

        // Send to Server 2
        std::string message = "Hello from Server 1";
        send(server1_socket, message.c_str(), message.size(), 0);
        sleep(1); 
    }
}

int main() {
    // Server 1 setup
    int server1_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server1_address;
    server1_address.sin_family = AF_INET;
    server1_address.sin_addr.s_addr = INADDR_ANY;
    server1_address.sin_port = htons(SERVER1_PORT);
    bind(server1_socket, (struct sockaddr *)&server1_address, sizeof(server1_address));
    listen(server1_socket, 3);

    std::cout << "Server 1 is waiting for connection on port " << SERVER1_PORT << std::endl;

    // Accept connection from Server 2
    int new_socket = accept(server1_socket, (struct sockaddr *)NULL, NULL);
    if (new_socket < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        return -1;
    }

    // Start the thread for communication
    std::thread t1(server1_thread, new_socket);
    t1.join();

    return 0;
}
