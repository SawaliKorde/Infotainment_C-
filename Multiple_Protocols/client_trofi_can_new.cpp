#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include<thread>
#include<fstream>
#include<semaphore.h>

using namespace std;


sem_t sem;
string fileName;

const char* shm_name = "/my_shared_memory";
const int shm_size = 4096;


void readSharedMemory(const string& protocol, int interval) {
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cerr << "Shared memory opening failed for " << protocol << "\n";
        return;
    }
	
    
    while (true) {
       sem_wait(&sem);
        void* ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            cerr << "Mapping shared memory failed for " << protocol << "\n";
            return;
        }
	sem_post(&sem);
        string message(static_cast<char*>(ptr));
        
        
        if (message.find("Protocol: " + protocol) != string::npos) {
            cout << protocol << " Client Read:\n" << message << "\n";
            
            ofstream file(fileName, ios::out | ios::app);
	  if(!file){
	  cerr<<"Cannot open the file to write."<<endl;
    		}else{
		    file << message <<endl;
		    file.flush();
		    file.close();
            }
        }
        munmap(ptr, shm_size);
        //sem_post(&sem);
        chrono::seconds interval_duration(interval);
        this_thread::sleep_for(interval_duration);
    }
    
}


int main() {
    

    string protocol;
    cout << "Available protocols TCP, CAN, PROFIBUS"<<endl;
    cout << "Enter protocol type"<<endl;
    cin >> protocol;
    ofstream outfile;
    int interval; 

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
    fileName = "client_" + protocol + ".txt";
    sem_init(&sem, 0, 0);
    
    thread readThread (readSharedMemory, protocol, interval);
    sem_post(&sem); //it will release main thread after read thread takes the signal.
    readThread.join();

    return 0;
}
