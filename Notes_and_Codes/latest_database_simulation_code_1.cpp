#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <string>
#include <condition_variable>
#include <vector>
#include <map>
#include <atomic>
#include <sstream>
#include <limits>
#include <regex>
#include <iomanip> // For setprecision
#include <queue>
#include <functional>
#include <algorithm>
#include <future>
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
};

struct Employee {
    int empId;
    string empName;
    double empSalary;
    string empDesignation;
    string empDate;

    Employee(int id, const string &name, double salary, const string &designation, const string &date)
        : empId(id), empName(name), empSalary(salary), empDesignation(designation), empDate(date) {}

    Employee() : empId(0), empName(""), empSalary(0.0), empDesignation(""), empDate("") {}
};

mutex coutMutex;
void logThreadStatus(const std::string &operation, const std::string &status) {
    std::lock_guard<std::mutex> guard(coutMutex); // Separate mutex for cout
    std::cout << "Thread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;
    std::cout.flush();
}

// Abstract class
class EmpDatabase {
public:
    virtual void createData(ofstream &filename, mutex &fileMutex) = 0;
    virtual void readData(mutex &fileMutex) = 0;
    virtual void retrieveData(int id, mutex &fileMutex) = 0;
    virtual void updateData(ofstream &filename, mutex &fileMutex) = 0;
    virtual void deleteData(int id, ofstream &filename, mutex &fileMutex) = 0;

    EmpDatabase() {}
};

class Operation : public EmpDatabase {
private:
    string filename;
    map<int, Employee> Emp;

public:
    Operation(const string &name) : filename(name) {}

    void setEmpData(const map<int, Employee> &data) {
        Emp = data;
    }

    map<int, Employee> getEmpData() {
        return Emp;
    }

    bool isValidInt(int &input) {
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
        return true;
    }

    bool isValidDouble(double &input) {
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
        return true;
    }

    bool isLeapYear(int year) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            return true;
        }
        return false;
    }

    bool isValidDate(const string &date) {
        regex dateRegex("^\\d{2}-\\d{2}-\\d{4}$");
        if (!regex_match(date, dateRegex)) {
            return false; // Invalid format
        }

        // Extract day, month, year from the date
        int day, month, year;
        char dash1, dash2;
        stringstream ss(date);
        ss >> day >> dash1 >> month >> dash2 >> year;

        // Check for valid month (1-12)
        if (month < 1 || month > 12) {
            return false;
        }

        // Check for valid day based on month
        int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2) { // February
            if (isLeapYear(year)) {
                daysInMonth[1] = 29; // Leap year, February has 29 days
            }
        }

        if (day < 1 || day > daysInMonth[month - 1]) {
            return false; // Invalid day for the month
        }

        return true; // Valid date
    }

    bool isValidNameOrDesignation(const string &input) {
        regex nameOrDesigRegex("^[A-Za-z ]+$");
        return regex_match(input, nameOrDesigRegex);
    }

    void createData(ofstream &file, mutex &fileMutex) override {
        int empId;
        string empName, empDesignation, empDate;
        double empSalary;

        // Prompt and validate Employee ID
        cout << "Enter Employee ID: ";
        cin >> empId;
        if (!isValidInt(empId)) {
            cout << "Invalid input for Employee ID. Please try again.\n";
            return;
        }
        // Ensure Employee ID is unique
        if (Emp.find(empId) != Emp.end()) {
            cerr << "Error: Employee ID " << empId << " already exists. Please enter a unique ID.\n";
            return;
        }

        // Validate Employee Name
        cout << "Enter Employee Name: ";
        cin.ignore(); // Ignore the newline character left by previous input
        getline(cin, empName);
        if (!isValidNameOrDesignation(empName)) {
            cout << "Employee Name can only contain alphabets and spaces. Please try again.\n";
            return;
        }

        // Validate Employee Designation
        cout << "Enter Employee Designation: ";
        getline(cin, empDesignation);
        if (!isValidNameOrDesignation(empDesignation)) {
            cout << "Employee Designation can only contain alphabets and spaces. Please try again.\n";
            return;
        }

        // Validate Employee Salary
        cout << "Enter Employee Salary: ";
        cin >> empSalary;
        if (!isValidDouble(empSalary)) {
            cout << "Invalid input for Salary. Please try again.\n";
            return;
        }

        // Validate Joining Date format (DD-MM-YYYY)
        cout << "Enter Joining Date (DD-MM-YYYY): ";
        cin.ignore(); // Ignore the newline character left by previous input
        getline(cin, empDate);
        if (!isValidDate(empDate)) {
            cout << "Invalid date format. Please use DD-MM-YYYY.\n";
            return;
        }

        // Add the new employee to the map
        Emp[empId] = Employee(empId, empName, empSalary, empDesignation, empDate);

        // Save data to the file
        saveData(file, fileMutex);

        cout << "Employee Data created successfully.\n";
    }

    void saveData(ofstream &file, mutex &fileMutex) {
    logThreadStatus("Save", "Started");
    lock_guard<mutex> lock(fileMutex);
    
    // Open file in truncate mode to rewrite entire file
    ofstream outFile(filename, ios::out | ios::trunc);
    if (!outFile.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    // Write each employee's data to the file
    for (const auto &entry : Emp) {
        const Employee &emp = entry.second;
        outFile << emp.empId << ","
                << emp.empName << ","
                << fixed << setprecision(2) << emp.empSalary << ","
                << emp.empDesignation << ","
                << emp.empDate << "\n";
    }

    outFile.close();
    logThreadStatus("Save", "Completed");
}

    void loadData() {
        logThreadStatus("Load", "Started");
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file!" << endl;
            return;
        }

        string line;
        cout << "Employee Records:\n";
        while (getline(file, line)) {
            stringstream ss(line);
            Employee emp;
            string temp;
            getline(ss, temp, ',');
            emp.empId = stoi(temp);
            getline(ss, emp.empName, ',');
            getline(ss, temp, ',');
            emp.empSalary = stod(temp);
            getline(ss, emp.empDesignation, ',');
            getline(ss, emp.empDate, ',');

            Emp[emp.empId] = emp;
        }
        file.close();
    }

    void readData(mutex &fileMutex) override {
        logThreadStatus("Read", "Started");
        lock_guard<mutex> lock(fileMutex);
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file!" << endl;
            return;
        }

        string line;
        cout << "Employee Records:\n";
        while (getline(file, line)) {
            stringstream ss(line);
            Employee emp;
            string temp;
            getline(ss, temp, ',');
            emp.empId = stoi(temp);
            getline(ss, emp.empName, ',');
            getline(ss, temp, ',');
            emp.empSalary = stod(temp);
            getline(ss, emp.empDesignation, ',');
            getline(ss, emp.empDate, ',');

            cout << "Employee ID: " << emp.empId << ", Name: " << emp.empName
                 << ", Salary: " << fixed << setprecision(2) << emp.empSalary
                 << ", Designation: " << emp.empDesignation
                 << ", Joining Date: " << emp.empDate << endl;
        }
        file.close();
    }

    void retrieveData(int ID, mutex &fileMutex) override {
        logThreadStatus("Retrieve", "Started");
        lock_guard<mutex> lock(fileMutex);
        if (Emp.find(ID) != Emp.end()) {
            Employee emp = Emp[ID];
            cout << "Employee found: \n";
            cout << "ID: " << emp.empId << "\n";
            cout << "Name: " << emp.empName << "\n";
            cout << "Salary: " << emp.empSalary << "\n";
            cout << "Designation: " << emp.empDesignation << "\n";
            cout << "Join Date: " << emp.empDate << "\n";
        } else {
            cout << "No Employee ID Found: " << ID << endl;
        }
    }

    void updateData(ofstream &file, mutex &fileMutex) override {
        logThreadStatus("Update", "Started");

        int id;
        cout << "Enter Employee ID to update data: ";
        cin >> id;
        if (!isValidInt(id)) {
            cout << "Invalid Employee ID. Please try again.\n";
            return;
        }

        if (Emp.find(id) == Emp.end()) {
            cout << "No matching ID available in database!\n";
            return;
        }

        int choice;
        cout << "\n1. Name\n2. Salary\n3. Designation\n4. Date\n";
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1: {
            string name;
            cout << "Enter new name: ";
            getline(cin, name);
            if (isValidNameOrDesignation(name)) {
                Emp[id].empName = name;
            }
            break;
        }
        case 2: {
            double salary;
            cout << "Enter new Salary: ";
            cin >> salary;
            if (isValidDouble(salary)) {
                Emp[id].empSalary = salary;
            }
            break;
        }
        case 3: {
            string designation;
            cout << "Enter new designation: ";
            getline(cin, designation);
            if (isValidNameOrDesignation(designation)) {
                Emp[id].empDesignation = designation;
            }
            break;
        }
        case 4: {
            string date;
            cout << "Enter new date (YYYY-MM-DD): ";
            getline(cin, date);
            if (isValidDate(date)) {
                Emp[id].empDate = date;
            }
            break;
        }
        default:
            cout << "Invalid choice\n";
            return;
        }
        saveData(file, fileMutex);
    }

  /*  void deleteData(int id, ofstream &file, mutex &fileMutex) override {
        logThreadStatus("Delete", "Started");
        lock_guard<mutex> lock(fileMutex);
        if (Emp.find(id) != Emp.end()) {
            Emp.erase(id);
            cout << "Employee with ID " << id << " has been deleted from the database.\n";
            saveData(file, fileMutex); // Save changes to the file
        } else {
            cout << "No Employee ID Found: " << id << endl;
        }
    }*/
    
    void deleteData(int id, ofstream &file, mutex &fileMutex) override {
    	logThreadStatus("Delete", "Started");
    	
    	lock_guard<mutex> lock(fileMutex);
    	if (Emp.find(id) != Emp.end()) {
            Emp.erase(id);
            cout << "Employee with ID " << id << " has been deleted from the database.\n";
            
            ofstream outFile(filename, ios::out);
            if(!outFile){
            	cerr << "Error opening file for writing.\n";
            	return;
            
            }
            for(const auto &entry : Emp){
            	const Employee &emp = entry.second;
            	outFile << emp.empId << ","
            		<< emp.empName << ","
            		<< fixed << setprecision(2) << emp.empSalary << ","
            		<< emp.empDesignation << ","
            		<< emp.empDate << "\n";
            
            }
            
            outFile.close();
            logThreadStatus("Delete", "Completed Successfully");    
    }
    else{
    	cout << "No Employee ID Found: " << id << endl;
    }
   }
    
};

int main() {
    string filename = "employee_data.csv";
    mutex fileMutex;
    Operation empOps(filename);
    ThreadPool pool(4); // Create a thread pool with 4 workers
    
      {
        lock_guard<mutex> lock(fileMutex);
        ofstream createFile(filename, ios::app);
        createFile.close();
    }

    empOps.loadData();  // Load existing data into memory
    
    
    vector<pair<int, function<void()>>> operations; // Store operations for batch processing

    while (true) {
        cout << "\nEmployee Management System\n";
        cout << "1. Create Employee Data\n";
        cout << "2. Read Employees Data\n";
        cout << "3. Retrieve Employee by ID\n";
        cout << "4. Update Employee Data\n";
        cout << "5. Delete Employee Data\n";
        cout << "6. Execute Pending Operations\n";
        cout << "7. Exit\n";
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                operations.push_back({1, [&]() {
                    ofstream file(filename, ios::app);
                    empOps.createData(file, fileMutex);
                    file.close();
                }});
                cout << "Create operation queued.\n";
                break;
            }
            case 2: {
                operations.push_back({2, [&]() {
                    empOps.readData(fileMutex);
                }});
                cout << "Read operation queued.\n";
                break;
            }
            case 3: {
                int id;
                cout << "Enter Employee ID: ";
                cin >> id;
                operations.push_back({3, [&, id]() {
                    empOps.retrieveData(id, fileMutex);
                }});
                cout << "Retrieve operation queued.\n";
                break;
            }
            case 4: {
                operations.push_back({4, [&]() {
                    ofstream file(filename, ios::app);
                    empOps.updateData(file, fileMutex);
                    file.close();
                }});
                cout << "Update operation queued.\n";
                break;
            }
            case 5: {
                int id;
                cout << "Enter Employee ID to delete: ";
                cin >> id;
                operations.push_back({5, [&, id]() {
                    ofstream file(filename, ios::app);
                    empOps.deleteData(id, file, fileMutex);
                    file.close();
                }});
                cout << "Delete operation queued.\n";
                break;
            }
            case 6: {
                cout << "Executing all pending operations in parallel...\n";
                for (auto &op : operations) {
                    pool.enqueue(op.second);
                }
                operations.clear();
                break;
            }
            case 7: {
                cout << "Exiting the program.\n";
                return 0;
            }
            default:
                cout << "Invalid choice, please try again.\n";
        }
    }

    return 0;
}
