#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include<thread>
using namespace std;

const char* shm_name = "/my_shared_memory";
const int shm_size = 4096;

void readSharedMemory(const string& protocol, int interval) {
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cerr << "Shared memory opening failed for " << protocol << "\n";
        return;
    }

    while (true) {
        void* ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            cerr << "Mapping shared memory failed for " << protocol << "\n";
            return;
        }

        string message(static_cast<char*>(ptr));
        if (message.find("Protocol: " + protocol) != string::npos) {
            cout << protocol << " Client Read:\n" << message << "\n";
        }

        munmap(ptr, shm_size);
        chrono::seconds interval_duration(interval);
        this_thread::sleep_for(interval_duration);  // Sleep using chrono
    }
}

int main() {
    
    cout << "Available protocols TCP, CAN, PROFIBUS"<<endl;	
    string protocol;
    cout << "Enter protocol type"<<endl;
    cin >> protocol;
    
    int interval;

    // Assign protocol and interval based on the input argument
    if (protocol == "TCP") {
        interval = 1;
    } else if (protocol == "CAN") {
        interval = 2;
    } else if (protocol == "PROFIBUS") {
        interval = 5;
    } else {
        cerr << "Invalid protocol. Exiting.\n";
        return 1;
    }

    // Read shared memory based on the selected protocol
    readSharedMemory(protocol, interval);

    return 0;
}
