Sawali Korde <sawalikorde200@gmail.com>
	
9:42 PM (0 minutes ago)
	
to me
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <map>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

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
                        condition.wait(lock, [this] { return stop ||
!tasks.empty(); });
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
};

class Operation {
    // Define your operations as in the original code
    string filename;
    map<int, string> EmpData; // Example storage

public:
    Operation(const string &file) : filename(file) {}

    void createData(int empId, const string &empName) {
        lock_guard<mutex> lock(coutMutex);
        EmpData[empId] = empName;
        cout << "Created Employee: ID = " << empId << ", Name = " <<
empName << endl;
    }

    void displayData() {
        lock_guard<mutex> lock(coutMutex);
        cout << "Employee Database:\n";
        for (const auto &entry : EmpData) {
            cout << "ID: " << entry.first << ", Name: " << entry.second << endl;
        }
    }

private:
    static mutex coutMutex;
};

mutex Operation::coutMutex;

void inputHandler(queue<string> &inputs, mutex &inputsMutex,
atomic<bool> &running) {
    string input;
    while (running) {
        cout << "Enter a command (e.g., 'create <id> <name>' or 'display'): ";
        getline(cin, input);
        {
            lock_guard<mutex> lock(inputsMutex);
            inputs.push(input);
        }
        if (input == "exit") {
            running = false;
            break;
        }
    }
}

int main() {
    ThreadPool pool(4); // 4 threads
    Operation empOps("employee_data.csv");

    queue<string> inputs;
    mutex inputsMutex;
    atomic<bool> running(true);

    // Start the input thread
    thread inputThread(inputHandler, ref(inputs), ref(inputsMutex),
ref(running));

    while (running) {
        string command;
        {
            lock_guard<mutex> lock(inputsMutex);
            if (!inputs.empty()) {
                command = inputs.front();
                inputs.pop();
            }
        }

        if (!command.empty()) {
            stringstream ss(command);
            string action;
            ss >> action;

            if (action == "create") {
                int id;
                string name;
                ss >> id >> name;
                pool.enqueue([&empOps, id, name]() {
                    empOps.createData(id, name);
                });
            } else if (action == "display") {
                pool.enqueue([&empOps]() {
                    empOps.displayData();
                });
            } else if (action == "exit") {
                running = false;
            } else {
                cout << "Unknown command: " << command << endl;
            }
        }

        this_thread::sleep_for(chrono::milliseconds(100)); // Prevent
busy waiting
    }

    inputThread.join();
    return 0;
}

	

