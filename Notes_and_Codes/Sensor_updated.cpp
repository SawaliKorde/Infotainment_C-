/*
What this code does ?
This code simulates a system that fetches data from multiple devices, processes the data in parallel using threads, stores it in a CSV file, and performs analysis on the collected data. The devices follow different protocols (TCP, CAN, MODBUS, PROFIBUS) and generate simulated sensor readings. The data is stored in a file and periodically analyzed to show trends in the values from each sensor.

Overall Flow
Simulated Data Collection: Devices generate random sensor data at regular intervals.
Threading: A thread pool handles multiple devices concurrently to fetch data.
File Writing: Data is written to a CSV file in a thread-safe manner.
Analysis: The CSV file is periodically read, and an analysis of the data (including trends, min, max, and current values) is displayed.
User Exit: The program continues collecting and analyzing data until the user presses a key to exit.


Flow Chart (Text-Based Representation)
+-------------------------------+
|        Start Program           |
+-------------------------------+
             |
             v
+-------------------------------+
| Initialize random number seed  |
+-------------------------------+
             |
             v
+-------------------------------+
| Create devices and thread pool |
+-------------------------------+
             |
             v
+-------------------------------+
| Open CSV file for writing      |
+-------------------------------+
             |
             v
+-------------------------------------------+
|  Enter main loop (until key is pressed)  |
+-------------------------------------------+
             |
             v
+-------------------------------------------+
|  Enqueue data fetch tasks (for each device) |
+-------------------------------------------+
             |
             v
+-------------------------------------------+
| Sleep for 2 seconds to simulate interval  |
+-------------------------------------------+
             |
             v
+-------------------------------------------+
| After 2 seconds:                         |
| - Close CSV file for writing             |
| - Reopen CSV file for reading            |
+-------------------------------------------+
             |
             v
+-------------------------------------------+
|  Analyze and display the collected data  |
+-------------------------------------------+
             |
             v
+-------------------------------------------+
|  Display message: "Fetching and analyzing data..." |
+-------------------------------------------+
             |
             v
+-------------------------------+
| Check if a key was pressed    |
+-------------------------------+
             |
             v
   +---------+---------------------+
   |         | No                  | Yes
   v         |                     v
+-------------------------+  +-----------------+
| Continue loop            |  | Exit program    |
|                         |  +-----------------+
+-------------------------+ 
             |
             v
+-------------------------------+
| Clean up and delete devices   |
+-------------------------------+
             |
             v
+-------------------------------+
|        End Program            |
+-------------------------------+


Functional Breakdown
Initialization:

	A random number seed is initialized using srand(time(0)) to ensure random values are generated each time the program runs.
	Devices are created using polymorphism. There are four types of devices: TCPDevice, CANDevice, MODBUSDevice, and PROFIBUSDevice. Each device generates simulated data.
	A ThreadPool is created to handle concurrent fetching of data from multiple devices. Each device will generate data in a separate thread.

File Handling:
	A CSV file named sensor_data.csv is opened in append mode. This allows new data to be added without overwriting the file contents.
	The CSV file has a header: Device ID, Time, Protocol, Raw Count, Scaled Value.

Main Loop:

	The main loop runs continuously until the user presses a key (detected using the kbhit() function).
	Each device's fetchData method is invoked in parallel using the thread pool. This method simulates generating sensor data, which is written to the CSV file.
	After 2 seconds, the CSV file is closed, reopened for reading, and the data is analyzed.

Data Analysis:

	The data is read line by line from the CSV file and parsed into device-specific values.
	For each device, the minimum, maximum, and current values are computed.
	A simple "trend" line is generated based on the last 10 values, showing whether the value is increasing or decreasing.
	The analysis is printed to the console for each sensor.

Exit:
	The loop continues fetching and analyzing data until the user presses a key. Once a key is detected, the program cleans up the device objects and exits.

Thread Pool Design
	The thread pool is designed to handle concurrent tasks:

Enqueue Tasks: 
	Each device’s data fetching function is enqueued in the thread pool, allowing the devices to work in parallel.
Task Execution: 
	A separate worker thread processes each task, ensuring that data fetching happens concurrently for multiple devices.
Synchronization: 
	A mutex is used to prevent race conditions when writing data to the CSV file. The condition_variable and tasksMutex are used to manage task scheduling and completion.
Non-blocking Key Detection (kbhit)
	The kbhit function uses termios to perform non-blocking keyboard input detection. It checks if a key has been pressed without waiting for the user to press enter. If a key is detected, the program exits the main loop.

Data Structure and Analysis
Device Data Structure: 
		A DeviceData struct holds the information for each device, including its ID, time, protocol, raw count, and scaled value.
Data Parsing: 
		The CSV file is parsed line-by-line to extract device data, and the min_element and max_element functions are used to determine the min and max values for each device's readings.

*/




#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <functional>
#include <atomic>
#include <deque>
#include <condition_variable>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <queue>      // Include the necessary header for std::queue
#include <map>        // Include the necessary header for std::map
#include <termios.h>  // Include termios for non-blocking keyboard input
#include <algorithm>  // Include algorithm for min_element and max_element

using namespace std;

// Device Protocol Enum
enum Protocol {
    TCP,
    CAN,
    MODBUS,
    PROFIBUS,
    BACNET
};

// Device Data Structure
struct DeviceData {
    int deviceid;
    string devicetime;
    Protocol deviceprotocol;
    int devicerawcount;
    double devicescalevalue;

    DeviceData(int id, Protocol protocol, int rawCount) :
        deviceid(id), deviceprotocol(protocol), devicerawcount(rawCount), devicescalevalue(rawCount / 1000.0) {
        devicetime = "2024-12-23 12:00:00";  // For simplicity, using a fixed timestamp
    }
};
bool isDeviceDataValid(int deviceid , int devicerawcount , Protocol protocol)
{
     if(deviceid < 0 || deviceid > 20)
     {
        cout<<"Invalid Device ID : "<<deviceid<<endl;
        return false;
     }
     if(devicerawcount<0)
     {
        cout<<"Invalid invalid Raw count : "<<deviceid<<devicerawcount<<endl;
        return false;
     }
     if(protocol != TCP && protocol != CAN && protocol != MODBUS && protocol != PROFIBUS && protocol != BACNET )
     {
        cout<<"Invalid Device Protocol  : "<<protocol<<endl;
        return false;
     }
     return true;


}
// Abstract Device Class
class Device {
public:
    virtual void fetchData(ofstream& file, mutex& fileMutex) = 0;
    virtual ~Device() = default;
};

// TCP Device
class TCPDevice : public Device {
public:
    void fetchData(ofstream& file, mutex& fileMutex) override {
          int rawcount = rand() % 5000;
          int deviceid = rand() % 10;
          Protocol protocol = TCP;
          if(isDeviceDataValid(deviceid,rawcount,protocol)){
          DeviceData data(deviceid,protocol,rawcount);
          /*DeviceData data(rand() % 10, TCP, rand() % 5000);  // Simulate data*/
         lock_guard<mutex> lock(fileMutex);  // Locking to prevent race condition
         file << data.deviceid << "," << data.devicetime << "," << data.deviceprotocol << ","
             << data.devicerawcount << "," << data.devicescalevalue << endl;
             }
    }
};

// CAN Device
class CANDevice : public Device {
public:
    void fetchData(ofstream& file, mutex& fileMutex) override {
    int rawcount = rand() % 5000;
          int deviceid = rand() % 10;
          Protocol protocol = CAN;
          if(isDeviceDataValid(deviceid,rawcount,protocol)){
          DeviceData data(deviceid,protocol,rawcount);
       // DeviceData data(rand() % 10, CAN, rand() % 5000);  // Simulate data
          lock_guard<mutex> lock(fileMutex);  // Locking to prevent race condition
          file << data.deviceid << "," << data.devicetime << "," << data.deviceprotocol << ","
             << data.devicerawcount << "," << data.devicescalevalue << endl;
             }
    }
};

// MODBUS Device
class MODBUSDevice : public Device {
public:
    void fetchData(ofstream& file, mutex& fileMutex) override {
          int rawcount = rand() % 5000;
          int deviceid = rand() % 10;
          Protocol protocol = MODBUS;
          if(isDeviceDataValid(deviceid,rawcount,protocol)){
          DeviceData data(deviceid,protocol,rawcount);
         // DeviceData data(rand() % 10, MODBUS, rand() % 5000);  // Simulate data
        lock_guard<mutex> lock(fileMutex);  // Locking to prevent race condition
        file << data.deviceid << "," << data.devicetime << "," << data.deviceprotocol << ","
             << data.devicerawcount << "," << data.devicescalevalue << endl;
             }
    }
};

// PROFIBUS Device
class PROFIBUSDevice : public Device {
public:
    void fetchData(ofstream& file, mutex& fileMutex) override {
          int rawcount = rand() % 5000;
          int deviceid = rand() % 10;
          Protocol protocol = PROFIBUS;
          if(isDeviceDataValid(deviceid,rawcount,protocol)){
          DeviceData data(deviceid,protocol,rawcount);
       // DeviceData data(rand() % 10, PROFIBUS, rand() % 5000);  // Simulate data
          lock_guard<mutex> lock(fileMutex);  // Locking to prevent race condition
          file << data.deviceid << "," << data.devicetime << "," << data.deviceprotocol << ","
             << data.devicerawcount << "," << data.devicescalevalue << endl;
             }
    }
};

class BACNETDevice : public Device {
public:
    void fetchData(ofstream& file, mutex& fileMutex) override {
	int rawcount = rand() % 5000;
          int deviceid = rand() % 10;
          Protocol protocol = BACNET;
          if(isDeviceDataValid(deviceid,rawcount,protocol)){
          DeviceData data(deviceid,protocol,rawcount);
       // DeviceData data(rand() % 10, PROFIBUS, rand() % 5000);  // Simulate data
        lock_guard<mutex> lock(fileMutex);  // Locking to prevent race condition
        file << data.deviceid << "," << data.devicetime << "," << data.deviceprotocol << ","
             << data.devicerawcount << "," << data.devicescalevalue << endl;
    }
  }
};

// Thread Pool Class
class ThreadPool {
private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    mutex tasksMutex;
    condition_variable condition;
    atomic<bool> stop;

public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (!stop) {
                    function<void()> task;
                    {
                        unique_lock<mutex> lock(tasksMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            unique_lock<mutex> lock(tasksMutex);
            stop = true;
        }
        condition.notify_all();
        for (thread &worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }

    void enqueue(function<void()> task) {
        {
            unique_lock<mutex> lock(tasksMutex);
            if (stop) throw runtime_error("enqueue on stopped ThreadPool");
            tasks.push(move(task));
        }
        condition.notify_one();
    }

    void shutdown() {
        stop = true;
        condition.notify_all();
    }

    void waitForCompletion() {
        while (true) {
            unique_lock<mutex> lock(tasksMutex);
            if (tasks.empty()) break;
        }
    }
};

// Analyze and display the data
void analyzeAndDisplayData(ifstream& file) {
    map<int, vector<double>> deviceValues;
    string line;
    while (getline(file, line)) {
        // Simulating CSV parsing for simplicity
        size_t pos = 0;
        int deviceid;
        double value;
        for (int i = 0; i < 4; ++i) {
            pos = line.find(',');
            line = line.substr(pos + 1);
        }
        deviceid = stoi(line.substr(0, line.find(',')));
        value = stod(line.substr(line.find_last_of(',') + 1));

        deviceValues[deviceid].push_back(value);
    }

    // Display analysis in tabular form
    for (const auto& entry : deviceValues) {
        const vector<double>& values = entry.second;
        double minVal = *min_element(values.begin(), values.end());
        double maxVal = *max_element(values.begin(), values.end());
        double currentValue = values.back();

        // Create a trend line (last 10 values)
        string trendLine = "";
        string graph = "";
        for (int i = 0; i < min(10, (int)values.size()); ++i) {
            trendLine += (values[values.size() - 1 - i] > values[values.size() - 2 - i] ? '*' : '=');
        }
        for (const auto& val : values) {
            int stars = static_cast<int>((val - minVal) / (maxVal - minVal) * 10); // Scale to 10
            graph += string(stars, '*') + "\n";
        }

        // Output the analysis
        cout << "Sensor " << entry.first
             << " | Min: " << minVal
             << " | Max: " << maxVal
             << " | Current Value: " << currentValue
             << " | Trend: " << trendLine << endl;
        cout << "Graph (scaled to 10 units):\n" << graph << endl;
    }
}

// Non-blocking key detection function
bool kbhit() {
    struct termios oldt, newt;
    int ch;
    bool kbhit = true;

    tcgetattr(STDIN_FILENO, &oldt);
    //cout<<"tcgetattr.\n";
    newt = oldt;
    newt.c_lflag &= ~ICANON;
    newt.c_lflag &= ~ECHO;
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    //cout<<"tcsetattr1\n";
    ch = getchar();
    //cout<<"CH:"<<ch;
    if (ch != EOF) {
        ungetc(ch, stdin);
        kbhit = false;
	//cout<<"kbhit:"<<kbhit;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    //cout<<"tcsetattr2.\n";
    return kbhit;
}

int main() {
    srand(time(0));  // Seed for random number generation

    // Create devices and thread pool
    vector<Device*> devices = { new TCPDevice(), new CANDevice(), new MODBUSDevice(), new PROFIBUSDevice(), new BACNETDevice() };
    ThreadPool pool(4);  // Using 4 threads for example
    mutex fileMutex;  // Mutex for file access

    // Open the file for writing
    ofstream file("sensor_data.csv", ios::out | ios::app);
    if (!file.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    // Run the loop infinitely until a key is pressed
    while (!kbhit()) {
	//cout<<"kbhit while entered.";
        // Enqueue data fetch tasks
        for (auto& device : devices) {
            pool.enqueue([&file, &fileMutex, device]() -> void {
                device->fetchData(file, fileMutex);
		//cout<<"Input file1 getting read";
            });
        }

        // Sleep for 2 seconds to simulate data fetching interval
        this_thread::sleep_for(chrono::seconds(2));

        // After writing data, reopen the file for reading
        file.close();  // Close the output file stream
        ifstream inputFile("sensor_data.csv", ios::in);  // Reopen the file in input mode
        if (inputFile.is_open()) {
            analyzeAndDisplayData(inputFile);  // Pass the ifstream object to the analysis function
        }
	else {
		cerr<<"Failed to open the file";
	}
        inputFile.close();  // Close the input file after analyzing

        // Optional: Display message to indicate data fetch cycle
        cout << "Fetching and analyzing data..." << endl;
    }

    cout << "Key pressed, exiting..." << endl;

    // Clean up the devices
    for (auto& device : devices) {
        delete device;
    }

    return 0;
}


