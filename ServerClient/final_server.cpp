#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>

#define PORT 8080

using namespace std;

mutex client_mutex;
vector<int> client_sockets;
atomic<int> client_count(0);
atomic<bool> server_running(true);

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    {
        lock_guard<mutex> lock(client_mutex);
        client_sockets.push_back(client_socket);
        client_count++;
    }

    try {
        while (server_running) {
            memset(buffer, 0, sizeof(buffer));
            int valread = read(client_socket, buffer, sizeof(buffer) - 1);
            if (valread <= 0) {
                break;
            }

            buffer[valread] = '\0';
            cout << "Client " << client_socket << ": " << buffer << endl;

            if (string(buffer) == "exit") {
                cout << "Client " << client_socket << " disconnected." << endl;
                break;
            }

            // Check for server exit command
            cout << "Server response to client: " << endl;
            string response;
            getline(cin, response);

            if (response == "exit") {
                cout << "Server shutdown initiated." << endl;
                server_running = false;
                break;
            }

            if (send(client_socket, response.c_str(),
response.length(), 0) < 0) {
                cout << "Failed to send message to client " <<
client_socket << endl;
                break;
            }
        }
    } catch (const exception& e) {
        cout << "Error Handling Client " << client_socket << ": " <<
e.what() << endl;
    }

    // Clean up client socket
    {
        lock_guard<mutex> lock(client_mutex);
        auto it = find(client_sockets.begin(), client_sockets.end(),
client_socket);
        if (it != client_sockets.end()) {
            client_sockets.erase(it);
        }
        client_count--;
    }

    close(client_socket);

    if (client_count.load() == 0) {
        cout << "No active clients" << endl;
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    vector<thread> client_threads;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
sizeof(opt)) < 0) {
        cerr << "Setsockopt failed" << endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed" << endl;
        close(server_fd);
        return -1;
    }

    cout << "Server is listening on port " << PORT << endl;

    while (server_running) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address,
(socklen_t*)&addrlen);
        if (new_socket < 0) {
            cerr << "Accept failed" << endl;
            continue;
        }

        cout << "New client connected with socket: " << new_socket << endl;
        client_threads.emplace_back(handleClient, new_socket);
    }

    // Clean up when server exits
    cout << "Server shutting down..." << endl;

    // Close all client sockets
    {
        lock_guard<mutex> lock(client_mutex);
        for (int sock : client_sockets) {
            close(sock);
        }
        client_sockets.clear();
    }

    // Join all client threads
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    close(server_fd);
    return 0;
}
