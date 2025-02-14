
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <sstream>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include<semaphore.h>
#include<atomic>

using namespace std;

const char* shm_name = "/my_shared_memory";
const int shm_size = 4096;

sem_t mem;

void* ptr1;

enum Protocol{
TCP,
CANBUS,
PROFIBUS
};


double getRandomTemp() { return rand() % 50; }
double getRandomPressure() { return rand() % 100; }
double getRandomVoltage() { return rand() % 250; }
double getRandomFlowMetre() { return rand() % 30; }
double getRandomRPM() { return rand() % 70; }


void tcp() {
    stringstream message;
    message << "DevID: " << rand() % 5 << endl
            << "Protocol: TCP" << endl
            << "Device Name: Temperature" + to_string(rand() % 10) << endl
            << "Temperature: " << getRandomTemp() << endl
            << "Device Name: Pressure" + to_string(rand() % 10) << endl
            << "Pressure: " << getRandomPressure() << endl
            << "Bus ID: " << rand() % 5 << "\n\n";
      
    sem_wait(&mem);
    cout<<"Reading data from SM"<<endl;
    memcpy(ptr1, message.str().c_str(), message.str().length() + 1);
    sem_post(&mem);
    cout<<"Writing data to SM"<<endl;
    cout << "[SERVER] TCP data written to shared memory:\n"<<message.str() << endl;
     
    
}

void canBus() {
   
    stringstream message;
    message << "DevID: " << rand() % 5 << endl
            << "Protocol: CANBUS" << endl
            << "Device Name: Voltage" + to_string(rand() % 10) << endl
            << "Voltage: " << getRandomVoltage() << endl
            << "Device Name: Flowmeter" + to_string(rand() % 10) << endl
            << "Flow Meter: " << getRandomFlowMetre() << endl
            
            << "Bus ID: " << rand() % 5 << "\n\n";
     sem_wait(&mem);
     cout<<"Reading data from SM"<<endl;
    memcpy(ptr1, message.str().c_str(), message.str().length() + 1);
     sem_post(&mem);
     cout<<"Writing data to SM"<<endl;
     cout << "[SERVER] CANBUS data written to shared memory:\n"<<message.str() << endl; 
    
}

void profiBus() {
   
    stringstream message;
    message << "DevID: " << rand() % 5 << endl
            << "Protocol: PROFIBUS" << endl
            << "Device Name: RPM" + to_string(rand() % 10) << endl
            << "RPM: " << getRandomRPM() << endl
            << "Device Name: Voltage" + to_string(rand() % 10) << endl
            << "Voltage: " << getRandomVoltage() << endl
            << "Bus ID: " << rand() % 5 << "\n\n";
     sem_wait(&mem);
     cout<<"Reading data from SM"<<endl;
    memcpy(ptr1, message.str().c_str(), message.str().length() + 1);
     sem_post(&mem);
     cout<<"Writing data to SM"<<endl;
     cout << "[SERVER] PROFIBUS data written to shared memory:\n"<<message.str() << endl;
     
}


void writeDataToMemory() {
   size_t total_memory_used = 0;
   
   	
    while (true) {
         cout << "\n-----------------------------------" << endl;
         cout << "Current memory usage : : "<< total_memory_used << "/" << shm_size << "bytes" << endl;
         cout << "\n-----------------------------------" << endl;
         std::cout << "\nThread [" << std::this_thread::get_id() << "] - "<< std::endl; 
        Protocol selectedProtocol = static_cast<Protocol>(rand() % 3);
        switch (selectedProtocol) {
            case TCP:
                tcp();
                total_memory_used+= strlen((char*)ptr1);
                break;
            case CANBUS:
                canBus();
                total_memory_used+= strlen((char*)ptr1);
                break;
            case PROFIBUS:
                profiBus();
                total_memory_used+= strlen((char*)ptr1);
                break;
        }
        if(total_memory_used >= shm_size){
	 cout <<"Limit reached --->  Resetting the size"<<endl;
	 sem_wait(&mem);
	  memset(ptr1,0,shm_size); // put this change
	 total_memory_used = 0;
	
    	sem_post(&mem);
	}
	 
        this_thread::sleep_for(chrono::seconds(3));
    }
}



int main() {
    
    srand(time(0));
    sem_init(&mem,0,1); //semaphore initialization
      	
    
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "Shared memory creation failed\n";
        return 1;
    }

    
    if (ftruncate(shm_fd, shm_size) == -1) {
        cerr << "Failed to set shared memory size\n";
        return 1;
    }

    
    ptr1 = mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr1 == MAP_FAILED) {
        cerr << "Mapping shared memory failed\n";
        return 1;
    }

    cout << "Server started! Writing data to shared memory...\n";

  
    thread writer_thread(writeDataToMemory);
    writer_thread.join();
    sem_destroy(&mem);
   
   munmap(ptr1, shm_size);
   shm_unlink(shm_name);

    return 0;
}
