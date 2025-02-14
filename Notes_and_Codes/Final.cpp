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
#include <chrono>
#include <sstream>
#include <limits>
#include <regex>
#include <iomanip> // For setprecision
#include <queue>
#include <functional>
#include <algorithm>
#include <future>
#include <unistd.h>
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

    //setter
    void setEmpData(const map<int, Employee> &data) {
        Emp = data;
    }

    //getter
    map<int, Employee> getEmpData() {
        return Emp;
    }

    //integer input validation(ID)
    bool isValidInt(int &input) {
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
        return true;
    }

    //double input validation(salary)
    bool isValidDouble(double &input) {
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
        return true;
    }

    //leap year validation
    bool isLeapYear(int year) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            return true;
        }
        return false;
    }

    //date input validation
    bool isValidDate(const string &date) {
        regex dateRegex("^\\d{2}-\\d{2}-\\d{4}$");
        if (!regex_match(date, dateRegex)) {
            return false; // Invalid format
        }

        int day, month, year;
        char dash1, dash2;
        stringstream ss(date);
        ss >> day >> dash1 >> month >> dash2 >> year;

        if (month < 1 || month > 12) {
            return false;
        }

        // Check for valid day based on month
        int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2) { 
            if (isLeapYear(year)) {
                daysInMonth[1] = 29; // Leap year, February has 29 days
            }
        }

        if (day < 1 || day > daysInMonth[month - 1]) {
            return false;
        }

        return true; 
    }

    //name & designation validation
    bool isValidNameOrDesignation(const string &input) {
        regex nameOrDesigRegex("^[A-Za-z ]+$");
        return regex_match(input, nameOrDesigRegex);
    }

    //create employee data function
    void createData(ofstream &file, mutex &fileMutex) override {
        int empId;
        string empName, empDesignation, empDate;
        double empSalary;

        cout << "Enter Employee ID: ";
        cin >> empId;
        if (!isValidInt(empId)) {
            cerr << "Invalid input for Employee ID. Please try again.\n";
            return;
        }
       
        if (Emp.find(empId) != Emp.end()) {
            cerr << "Error: Employee ID " << empId << " already exists. Please enter a unique ID.\n";
            return;
        }

        cout << "Enter Employee Name: ";
        cin.ignore();
        getline(cin, empName);
        if (!isValidNameOrDesignation(empName)) {
            cerr << "Employee Name can only contain alphabets and spaces. Please try again.\n";
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

        saveData(file, fileMutex);

        cout << "Employee Data created successfully.\n";
    }

   //saving the data into the file after operation
   void saveData(ofstream &file, mutex &fileMutex) {
	    logThreadStatus("Save", "Started");
	    lock_guard<mutex> lock(fileMutex);
	   
	    ofstream outFile(filename, ios::trunc);
	    if (!outFile) {
		cerr << "Error opening file for writing.\n";
		return;
	    }
	    
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

    //loading the data after file opening
    void loadData() {
        logThreadStatus("Load", "Started");
        ifstream file(filename);
        if (!file.is_open()) {
            //cerr << "Error opening file!" << endl;
            return;
        }

        string line;
        //cout << "Employee Records:\n";
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

    //reading the content from the file
    void readData(mutex &fileMutex) override {
        logThreadStatus("Read", "Started");
        lock_guard<mutex> lock(fileMutex);
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file!" << endl;
            return;
        }
        
        cout << "Employee Records:\n";
		cout << "\n+---------+----------------+----------------+---------------------+--------------+\n";
		std::cout << "| " << std::setw(8) << left << "ID"
                  << "| " << std::setw(15) << left<< "Name"
                  << "| " << std::setw(15)<< left << "Salary"
                  << "| " << std::setw(20)<< left << "Designation"
                  << "| " << std::setw(10)<< left << "Joining Date" << " |" <<std::endl;
		cout << "+---------+----------------+----------------+---------------------+--------------+\n";

        string line;
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
			cout <<"| " << setw(8) << left << emp.empId 
				 <<"| " << setw(15) << left << emp.empName
                 <<"| " << setw(15) << right  << fixed << setprecision(2) << emp.empSalary 
                 <<"| " << setw(20) << left  << emp.empDesignation
                 <<"| " << setw(10) << left  << emp.empDate << "   |" << endl;	 
        }
		cout << "+---------+----------------+----------------+---------------------+--------------+\n";
        file.close();
    }

    //retrieveing the Employee detial
    void retrieveData(int ID, mutex &fileMutex) override {
        logThreadStatus("Retrieve", "Started");
        lock_guard<mutex> lock(fileMutex);
        
        if (Emp.find(ID) != Emp.end()) {
        	cout << "\nEmployee Record:\n";
		cout << "\n+---------+----------------+----------------+---------------------+--------------+\n";
		std::cout << "| " << std::setw(8) << left << "ID"
                  << "| " << std::setw(15) << left<< "Name"
                  << "| " << std::setw(15)<< left << "Salary"
                  << "| " << std::setw(20)<< left << "Designation"
                  << "| " << std::setw(10)<< left << "Joining Date" << " |" <<std::endl;
		cout << "+---------+----------------+----------------+---------------------+--------------+\n";
		
            	Employee emp = Emp[ID];	 
		cout <<"| " << setw(8) << left << emp.empId 
			<<"| " << setw(15) << left << emp.empName
                	<<"| " << setw(15) << right  << fixed << setprecision(2) << emp.empSalary 
                 	<<"| " << setw(20) << left  << emp.empDesignation
                 	<<"| " << setw(10) << left  << emp.empDate << "   |" << endl;
                 	
                  cout << "+---------+----------------+----------------+---------------------+--------------+\n";	
        } else {
            cout << "No matching ID available in database: " << ID << endl;
        }
        
       
    }

    //updating the employe details
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

        //switch case for which detail to update
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

    //deleting the employee details
    void deleteData(int id, ofstream &file, mutex &fileMutex) override {
	    logThreadStatus("Delete", "Started");
	    
	    lock_guard<mutex> lock(fileMutex);
	    
	    // Check if the employee exists in the database
	    if (Emp.find(id) != Emp.end()) {
		Emp.erase(id); 
		cout << "Employee with ID " << id << " has been deleted from the database.\n";
		
		ofstream outFile(filename, ios::out);
		
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
		cout << "No matching ID available in database: " << id << endl;
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
  
	void enqueueAndWait(function<void()> f) {
	    auto task = make_shared<packaged_task<void()>>(f);
	    auto future = task->get_future();

	    {
		unique_lock<mutex> lock(tasksMutex);
		if (stop) {
		    throw runtime_error("Cannot enqueue on stopped ThreadPool");
		}
		tasks.emplace([task]() { (*task)(); });
	    }
	    condition.notify_one();
	    future.wait(); // Wait for task completion
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
    shared_mutex empMutex;
    Operation op(filename);
    ThreadPool pool(4);

    op.loadData();
    cout << "\n";
    cout << "Welcome to ";
    cout.flush();
        for (int i = 1; i < 4; i++) {
            cout << ".";
            cout.flush();
            sleep(1);
            
        }
    cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
    //cout << "    *MULTITHREADED DATABASE SIMULATION*    \n";
    string str = "   *MULTITHREADED DATABASE SIMULATION*   \n";
    int len1 = str.length();
    for(int i = 0; i < len1; i++)
    {
    	cout << str[i];
    	cout.flush();
    	this_thread::sleep_for(chrono::milliseconds(50));
    }
    
     

    while (true) {
        cout << "\n+---------------------------------------+\n";
    	cout << "|        Database Operation Menu        |\n";
    	cout << "+---------------------------------------+\n";
    	cout << "|  1. Create Employee Database          |\n";
    	cout << "|  2. Read Employee Data                |\n";
    	cout << "|  3. Retrieve Employee Details         |\n";
    	cout << "|  4. Update Employee Details           |\n";
    	cout << "|  5. Delete Employee Details           |\n";
    	cout << "|  6. Exit                              |\n";
	cout << "+---------------------------------------+\n\n";
	cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                pool.enqueueAndWait([&]() {
                    lock_guard<shared_mutex> lock(empMutex);
                    ofstream file(filename, ios::app);
                    op.createData(file, fileMutex);
                    file.close();
                });
                break;
            }

            case 2: {
                pool.enqueueAndWait([&]() {
                    shared_lock<shared_mutex> lock(empMutex);
                    op.readData(fileMutex);
                });
                break;
            }

            case 3: {
                int id;
                cout << "Enter Employee ID: ";
                cin >> id;
                pool.enqueueAndWait([&, id]() {
                    shared_lock<shared_mutex> lock(empMutex);
                    op.retrieveData(id, fileMutex);
                });
                break;
            }

            case 4: {
                pool.enqueueAndWait([&]() {
                    lock_guard<shared_mutex> lock(empMutex);
                    ofstream file(filename, ios::app);
                    op.updateData(file, fileMutex);
                    file.close();
                });
                break;
            }

            case 5: {
                int id;
                cout << "Enter Employee ID to delete: ";
                cin >> id;
                pool.enqueueAndWait([&, id]() {
                    lock_guard<shared_mutex> lock(empMutex);
                    ofstream file(filename, ios::app);
                    op.deleteData(id, file, fileMutex);
                    file.close();
                });
                break;
            }

            case 6:
            {
                cout << "Shutting down";
                string str2 = " threads and exiting the program.\n";
    		int len2 = str.length();
    		
    		cout.flush();
        	for (int i = 1; i < 4; i++) {
            		cout << ".";
            		cout.flush();
            		sleep(1);
            
        	}
    		cout << "\b\b\b";
    		cout.flush();
    		
    		for(int i = 0; i < len2; i++)
    		{
    			cout << str2[i];
    			cout.flush();
    			this_thread::sleep_for(chrono::milliseconds(50));
    		}
                pool.shutdown();
                pool.waitforCompletion();
                return 0;
            }

            default:
                cout << "Invalid choice, please try again.\n";
        }
    }

    return 0;
}

