#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>

#define PORT 8080

using namespace std;

void receiveMessages(int socket) {
    char buffer[1024] = {0};
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(socket, buffer, sizeof(buffer) - 1);
        if (valread <= 0) {
            cout << "Disconnected from server." << endl;
            break;
        }

        buffer[valread] = '\0';
        cout << "\n" << buffer << endl;
        cout << "Enter recipient ID and message (e.g., 1:Hello): ";
        cout.flush();
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address / Address not supported" << endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed" << endl;
        return -1;
    }

    cout << "Connected to the server. Start sending messages." << endl;

    thread receiver(receiveMessages, sock);

    string message;
    while (true) {
        cout << "Enter recipient ID and message (e.g., 1:Hello): ";
        getline(cin, message);

        if (message == "exit") {
            send(sock, message.c_str(), message.size(), 0);
            break;
        }

        send(sock, message.c_str(), message.size(), 0);
    }

    receiver.join();
    close(sock);
    return 0;
}
