#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

#define PORT 8080

using namespace std;

mutex client_mutex;
map<int, int> client_map; // Map client ID to socket
int client_id_counter = 1;

void handleClient(int client_socket, int client_id) {
    char buffer[1024] = {0};

    cout << "Client " << client_id << " connected." << endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(client_socket, buffer, sizeof(buffer) - 1);

        if (valread <= 0) {
            cout << "Client " << client_id << " disconnected." << endl;

            lock_guard<mutex> lock(client_mutex);
            client_map.erase(client_id);
            close(client_socket);
            return;
        }

        buffer[valread] = '\0';
        string message(buffer);

        if (message == "exit") {
            cout << "Client " << client_id << " exited." << endl;

            lock_guard<mutex> lock(client_mutex);
            client_map.erase(client_id);
            close(client_socket);
            return;
        }

        // Parse the message to extract target client ID and the actual message
        size_t pos = message.find(':');
        if (pos != string::npos) {
            int target_id = stoi(message.substr(0, pos));
            string msg_to_send = "Client " + to_string(client_id) + ": " + message.substr(pos + 1);

            lock_guard<mutex> lock(client_mutex);
            if (client_map.find(target_id) != client_map.end()) {
                int target_socket = client_map[target_id];
                send(target_socket, msg_to_send.c_str(), msg_to_send.size(), 0);
            } else {
                string error_msg = "Client " + to_string(target_id) + " not connected.";
                send(client_socket, error_msg.c_str(), error_msg.size(), 0);
            }
        } else {
            string error_msg = "Invalid message format. Use client_id:message";
            send(client_socket, error_msg.c_str(), error_msg.size(), 0);
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed" << endl;
        return -1;
    }

    cout << "Server started. Waiting for connections..." << endl;

    vector<thread> threads;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        if (new_socket < 0) {
            cerr << "Failed to accept client" << endl;
            continue;
        }

        lock_guard<mutex> lock(client_mutex);
        int client_id = client_id_counter++;
        client_map[client_id] = new_socket;

        threads.emplace_back(handleClient, new_socket, client_id);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    close(server_fd);
    return 0;
}
