#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    // Socket trying to create connection	
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    //	
    address.sin_family = AF_INET; // Setting address for IPv4
    address.sin_addr.s_addr = INADDR_ANY; // listen from any address
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        std::cerr << "Accept failed" << std::endl;
        return -1;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading
        int valread = read(new_socket, buffer, sizeof(buffer) - 1); // Leave space for null terminator
        if (valread <= 0) break;

        buffer[valread] = '\0'; // Null terminate the received string
        std::cout << "Client: " << buffer << std::endl;

        if (std::string(buffer) == "exit") {
            std::cout << "Connection closed by client." << std::endl;
            break;
        }

        std::string message;
        std::cout << "Server: ";
        std::getline(std::cin, message);
        send(new_socket, message.c_str(), message.size(), 0);

        if (message == "exit") {
            std::cout << "Server exiting." << std::endl;
           break;
        }
    }

    //close(new_socket);
    close(server_fd);
    return 0;
}
