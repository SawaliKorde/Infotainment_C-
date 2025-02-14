#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#define SERVER1_PORT 8080
#define SERVER2_PORT 8081

void server2_thread(int server2_socket) {
    char buffer[1024] = {0};
    while (true) {
        // Receive from Server 1
        int valread = read(server2_socket, buffer, 1024);
        if (valread > 0) {
            std::cout << "Server 2 received: " << buffer << std::endl;
        }

        // Send to Server 1
        std::string message = "Hello from Server 2";
        send(server2_socket, message.c_str(), message.size(), 0);
        sleep(1);
    }
}

int main() {
    // Server 2 setup
    int server2_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server1_address;
    server1_address.sin_family = AF_INET;
    server1_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server1_address.sin_port = htons(SERVER1_PORT);

    // Connect to Server 1
    std::cout << "Connecting to Server 1 on port " << SERVER1_PORT << std::endl;
    if (connect(server2_socket, (struct sockaddr *)&server1_address, sizeof(server1_address)) < 0) {
        std::cerr << "Failed to connect to Server 1" << std::endl;
        return -1;
    }

    // Start the thread for communication
    std::thread t2(server2_thread, server2_socket);
    t2.join();

    return 0;
}
