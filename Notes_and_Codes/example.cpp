#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>
#include <condition_variable>

std::mutex fileMutex;
std::mutex coutMutex;
std::condition_variable cv;
bool operationComplete = false;

// Utility function to log thread status
void logThreadStatus(const std::string& operation, const std::string& status) {
    std::lock_guard<std::mutex> guard(coutMutex);  // Separate mutex for cout
    std::cout << "Thread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;
    std::cout.flush();
}

// Function to create or add data
void createData(const std::string& data) {
    logThreadStatus("Create", "Started");
    {
        std::lock_guard<std::mutex> guard(fileMutex);
        std::ofstream file("data.txt", std::ios::app);
        if (file.is_open()) {
            file << data << std::endl;
            logThreadStatus("Create", "Completed Successfully");
        } else {
            logThreadStatus("Create", "Failed to Open File");
        }
    }
    
    // Signal completion
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        operationComplete = true;
        cv.notify_one();
    }
}

// Function to read data
void readData() {
    logThreadStatus("Read", "Started");
    {
        std::lock_guard<std::mutex> guard(fileMutex);
        std::ifstream file("data.txt");
        if (file.is_open()) {
            std::string line;
            {
                std::lock_guard<std::mutex> coutGuard(coutMutex);
                std::cout << "File Contents:\n";
                while (std::getline(file, line)) {
                    std::cout << line << std::endl;
                }
            }
            logThreadStatus("Read", "Completed Successfully");
        } else {
            logThreadStatus("Read", "Failed to Open File");
        }
    }
    
    // Signal completion
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        operationComplete = true;
        cv.notify_one();
    }
}

// Function to update data
void updateData(const std::string& oldData, const std::string& newData) {
    logThreadStatus("Update", "Started");
    bool updated = false;
    {
        std::lock_guard<std::mutex> guard(fileMutex);
        std::vector<std::string> lines;
        
        // Read phase
        std::ifstream file("data.txt");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line == oldData) {
                    lines.push_back(newData);
                    updated = true;
                } else {
                    lines.push_back(line);
                }
            }
            file.close();
        } else {
            logThreadStatus("Update", "Failed to Open File for Reading");
            return;
        }

        // Write phase
        std::ofstream outFile("data.txt");
        if (outFile.is_open()) {
            for (const auto& line : lines) {
                outFile << line << std::endl;
            }
            logThreadStatus("Update", updated ? "Completed Successfully" : "Data Not Found");
        } else {
            logThreadStatus("Update", "Failed to Open File for Writing");
        }
    }
    
    // Signal completion
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        operationComplete = true;
        cv.notify_one();
    }
}

// Function to delete data
void deleteData(const std::string& dataToDelete) {
    logThreadStatus("Delete", "Started");
    bool deleted = false;
    {
        std::lock_guard<std::mutex> guard(fileMutex);
        std::vector<std::string> lines;
        
        // Read phase
        std::ifstream file("data.txt");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line == dataToDelete) {
                    deleted = true;
                } else {
                    lines.push_back(line);
                }
            }
            file.close();
        } else {
            logThreadStatus("Delete", "Failed to Open File for Reading");
            return;
        }

        // Write phase
        std::ofstream outFile("data.txt");
        if (outFile.is_open()) {
            for (const auto& line : lines) {
                outFile << line << std::endl;
            }
            logThreadStatus("Delete", deleted ? "Completed Successfully" : "Data Not Found");
        } else {
            logThreadStatus("Delete", "Failed to Open File for Writing");
        }
    }
    
    // Signal completion
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        operationComplete = true;
        cv.notify_one();
    }
}

int main() {
    while (true) {
        std::cout << "\nChoose an operation:\n";
        std::cout << "1. Create/Add Data\n2. Read Data\n3. Update Data\n4. Delete Data\n5. Exit\n";
        int choice;
        std::cin >> choice;

        std::cin.ignore(); // Clear input buffer
        if (choice == 5) break;

        std::string data, newData;
        std::thread operation;

        // Reset completion flag
        operationComplete = false;

        switch (choice) {
            case 1:
                std::cout << "Enter data to add: ";
                std::getline(std::cin, data);
                operation = std::thread(createData, data);
                break;

            case 2:
                operation = std::thread(readData);
                break;

            case 3:
                std::cout << "Enter data to update: ";
                std::getline(std::cin, data);
                std::cout << "Enter new data: ";
                std::getline(std::cin, newData);
                operation = std::thread(updateData, data, newData);
                break;

            case 4:
                std::cout << "Enter data to delete: ";
                std::getline(std::cin, data);
                operation = std::thread(deleteData, data);
                break;

            default:
                std::cout << "Invalid choice, try again.\n";
                continue;
        }

        // Detach the thread
        operation.detach();

        // Wait for operation to complete
        {
            std::unique_lock<std::mutex> lock(coutMutex);
            cv.wait(lock, [] { return operationComplete; });
        }

        // Small delay to ensure all output is printed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
