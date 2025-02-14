#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <fstream>
#include <semaphore.h>

using namespace std;

sem_t sem;
const char* shm_name = "/my_shared_memory";
const int shm_size = 4096;

void readSharedMemory(const string& protocol, int interval, const string& fileName) {
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cerr << "Shared memory opening failed for " << protocol << "\n";
        return;
    }
    //File creation
    ofstream file(fileName, ios::out | ios::app);
    if (!file) {
        cerr << "Cannot open the file to write.\n";
        return;
    }

    while (true) {
        sem_wait(&sem);
        void* ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            cerr << "Mapping shared memory failed for " << protocol << "\n";
            return;
        }
        string message(static_cast<char*>(ptr));
        munmap(ptr, shm_size);
        sem_post(&sem);

        if (message.find("Protocol: " + protocol) != string::npos) {
            cout << protocol << " Client Read:\n" << message << "\n";
            file << message << endl; //writing to file
        }

        this_thread::sleep_for(chrono::seconds(interval));
    }
}

int main() {
    string protocol;
    cout << "Available protocols: TCP, CAN, PROFIBUS\n";
    cout << "Enter protocol type: ";
    cin >> protocol;

    int interval;
    string fileName;
    if (protocol == "TCP") {
        interval = 1;
        fileName = "client_TCP.txt"; //Giving file name accroding to the protocols
    } else if (protocol == "CAN") {
        interval = 2;
        fileName = "client_CAN.txt";
    } else if (protocol == "PROFIBUS") {
        interval = 5;
        fileName = "client_PROFIBUS.txt";
    } else {
        cerr << "Invalid protocol. Exiting.\n";
        return 1;
    }

    sem_init(&sem, 0, 1);
    thread readThread(readSharedMemory, protocol, interval, fileName);
    readThread.join();

    return 0;
}
