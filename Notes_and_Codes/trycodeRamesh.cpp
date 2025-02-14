#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <unordered_map>
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
#include<shared_mutex>


using namespace std;

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
    std::cout << "Thread - " << operation << ": " << status << std::endl;
                        //[" << std::this_thread::get_id() << "]
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
	    
	    // Open file in truncation mode (overwrite file content)
	    ofstream outFile(filename, ios::trunc); // This will clear the file before writing new data
	    if (!outFile) {
		cerr << "Error opening file for writing.\n";
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

	    //outFile.close(); // Close the file
	    logThreadStatus("Save", "Completed successfully.");
	    cout << "Data saved to file successfully.\n";
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

    void deleteData(int id, ofstream &file, mutex &fileMutex) override {
	    logThreadStatus("Delete", "Started");
	    
	    lock_guard<mutex> lock(fileMutex);
	    
	    // Check if the employee exists in the database
	    if (Emp.find(id) != Emp.end()) {
		Emp.erase(id); // Remove the employee with the given ID
		
		cout << "Employee with ID " << id << " has been deleted from the database.\n";
		
		// Open the file in truncation mode to overwrite the existing content
		ofstream outFile(filename, ios::out);  // This will clear the file and write new data
		
		if (!outFile) {
		    cerr << "Error opening file for writing.\n";
		    return;
		}
		
		// Save the updated data back to the file
		for (const auto &entry : Emp) {
		    const Employee &emp = entry.second;
		    outFile << emp.empId << ","
		            << emp.empName << ","
		            << fixed << setprecision(2) << emp.empSalary << ","
		            << emp.empDesignation << ","
		            << emp.empDate << "\n";
		}
		
		outFile.close();  // Close the file
		logThreadStatus("Delete", "Completed successfully.");
	    } else {
		cout << "No Employee ID Found: " << id << endl;
	    }
     }
};


//Thread Pool class
class ThreadPool
{
    private:
	vector<thread> workers;
	queue<function<void()>> tasks;
	mutex tasksMutex;
	condition_variable condition;
	atomic<bool> stop;
	
    public:
	ThreadPool(size_t numThreads) : stop(false){
	   for(size_t i = 0; i < numThreads; i++) {
		workers.emplace_back([this] {
			while(!stop) {
				function<void()> task;
				{
				     unique_lock<mutex> lock(tasksMutex);
				     condition.wait(lock, [this] { return stop || !tasks.empty(); });
				     if (stop && tasks.empty())  return;
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
	   
      /* void enqueue(function<void()> task) {
	   {
	   	unique_lock<mutex> lock(tasksMutex);
	   	if(stop) throw runtime_error("enqueue on stopped ThreadPool");
	   	tasks.push(move(task));
	   }
	   condition.notify_one();
        }*/
        
	template<typename F, typename... Args>
	auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
	    using return_type = decltype(f(args...));

	    auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
	    );

	    std::future<return_type> res = task->get_future();
	    {
		std::unique_lock<std::mutex> lock(shared_mutex);
		tasks.emplace([task]() { (*task)(); });
	    }
	    condition.notify_one();
	    return res;
	}


        void shutdown() {
	     stop = true;
	     condition.notify_all();
	}
	
	void waitforCompletion() {
	    while(true) {
		unique_lock<mutex> lock(tasksMutex);
		if(tasks.empty()) break;
	   }
	}
};

int main() {
    string filename = "EmpDatabase.csv";
    mutex fileMutex;
    shared_mutex empMutex; // Shared mutex for read/write control
    Operation op(filename);
    ThreadPool pool(4); // 4 threads in the pool

    op.loadData(); // Load existing data into memory

    while (true) {
        cout << "\nEmployee Management System\n";
        cout << "1. Create Employee Detail\n";
        cout << "2. Read Employees Details\n";
        cout << "3. Retrieve Employee by ID\n";
        cout << "4. Update Employee Details\n";
        cout << "5. Delete Employee\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                auto future = pool.enqueue([&] {
                    lock_guard<shared_mutex> lock(empMutex); // Exclusive lock for Create
                    ofstream file(filename, ios::app);
                    op.createData(file, fileMutex);
                    file.close();
                });
                future.get();
                break;
            }

            case 2: {
                auto future = pool.enqueue([&] {
                    shared_lock<shared_mutex> lock(empMutex); // Shared lock for Read
                    op.readData(fileMutex);
                });
                future.get();
                break;
            }

            case 3: {
                int id;
                cout << "Enter Employee ID: ";
                cin >> id;
                auto future = pool.enqueue([&] {
                    shared_lock<shared_mutex> lock(empMutex); // Shared lock for Retrieve
                    op.retrieveData(id, fileMutex);
                });
                future.get();
                break;
            }

            case 4: {
                auto future = pool.enqueue([&] {
                    lock_guard<shared_mutex> lock(empMutex); // Exclusive lock for Update
                    ofstream file(filename, ios::app);
                    op.updateData(file, fileMutex);
                    file.close();
                });
                future.get();
                break;
            }

            case 5: {
                int id;
                cout << "Enter Employee ID to delete: ";
                cin >> id;
                auto future = pool.enqueue([&] {
                    lock_guard<shared_mutex> lock(empMutex); // Exclusive lock for Delete
                    ofstream file(filename, ios::app);
                    op.deleteData(id, file, fileMutex);
                    file.close();
                });
                future.get();
                break;
            }

            case 6:
                cout << "Shutting down threads and exiting the program.\n";
                pool.shutdown();
                pool.waitforCompletion();
                return 0;

            default:
                cout << "Invalid choice, please try again.\n";
        }
    }

    return 0;
}




/*int main() {
    string filename = "EmpDatabase.csv"; 
    mutex fileMutex;
    Operation op(filename);
    ThreadPool pool(4);
    op.loadData();  // Load existing data into memory

    while (true) {
        cout << "\nEmployee Management System\n";
        cout << "1. Created Employee Detail \n";
        cout << "2. Read Employees Details \n";
        cout << "3. Retrieve Employee by ID\n";
        cout << "4. Update Employee Details \n";
        cout << "5. Delete Employee\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
                   case 1:
                         pool.enqueue([&] {
        		// Create an ofstream object here
                         ofstream file(filename, ios::app);  // Open file in append mode
                         op.createData(file, fileMutex);  // Pass file and mutex to the method
                         file.close();  // Close the file after the operation
                        });
                    break;

                case 2:
		    pool.enqueue([&]{ op.readData(fileMutex); });
                   
                break;

                case 3: {
                    int id;
                    cout << "Enter Employee ID: ";
                    cin >> id;
                    pool.enqueue([&] {op.retrieveData(id, fileMutex); });
                    break;
                 }
     
                case 4:
                {
                    pool.enqueue([&]{ ofstream file(filename, ios::app);  // Open file in append mode
                    op.updateData(file, fileMutex);   // Pass file and mutex to the method
                    file.close();
		});  // Close the file after the operation
                
                break;

                case 5:
                {
                    int id;
                    cout << "Enter Employee ID to delete: ";
                    cin >> id;
                    pool.enqueue([&]{
		       ofstream file(filename, ios::app);  // Open file in append mode
                       op.deleteData(id, file, fileMutex);  // Pass file and mutex to the method
                        file.close();  // Close the file after the operation
                  });
                    break;
                }
                break;

                case 6:
                    cout << "Exiting the program.\n";
                    return 0;
                default:
                    cout << "Invalid choice, please try again.\n";
        }
    }

    return 0;
 }
}*/
