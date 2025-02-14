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
#include<shared_mutex>
#include<filesystem>


using namespace std;
using namespace std::filesystem;

string dbpath = "/home/rps/Documents/"; //global


class DBStructure{
    public:
        virtual void create()=0;
        virtual void dbremove()=0;
        virtual void dblist(ofstream &folder)=0;
        
        
        virtual ~DBStructure() {}
};

class Database: public DBStructure {

  private:
     string folderName;

  public:
 
    Database(const string &folder) : folderName(folder) {}
    
    /*void create(string folderName) override{
    
    	//string folderName;
           //cout<<"Enter the name of the database to create.";
           //getline(cin, folderName);
           
        try{
              string dbpath = dbpath + folderName;
             if(!exists(dbpath))
            {
              create_directory(folderName);
              cout<<"Database created successfully" << folderName << endl;
            }
            else {
              cerr<<"Error: Already exists database.";
            }
         }
        catch (const filesystem_error& e) {
             cerr << "Error: " << e.what() << endl;
          }
       }*/
       
        void create(const string& dbName) {
        if (create_directory(dbName)) {
            cout << "Database '" << dbName << "' created successfully.\n";
        } else {
            cout << "Failed to create database. It might already exist.\n";
        }
    }
    
    void dbremove() override{
         
         string folderName;
         cout<<"Enter the Database to be remove: ";
         getline(cin, folderName);
         string deletepath = dbpath + folderName;
         //cout<<"delete path: "<<deletepath<<endl;
         if(!exists(deletepath)){
            cerr<<"Error: Database does not exists."<<endl;
         }else{
            //string dbpath = dbpath + folderName;
            remove_all(deletepath);
           cout<<"Database deleted successfully"<<endl;
         }
    }
    
    void dblist(ofstream &folder) override{
		int count = 0;
		
		try {
		// Iterate over the directory at the specified path
		for (const auto& entry : directory_iterator(dbpath)) {
				++count;
		    if (is_directory(entry.status())) {
		        // Print folder name if the entry is a directory
		         string filename = entry.path().filename().string();
		        if(filename[0] != '.')
		        {
		           cout<< count << ". " << filename << endl;
			}
		    }
		}
	    } catch (const filesystem_error& e) {
	 	cerr << "Error: " << e.what() << endl;
	    }
    }
};

class Table: public DBStructure{
   private:
   	string folderName;
   	string fileName;	

   public:
      Table(const string &folder, const string &file) : folderName(folder), fileName(file) {}
    
      
      void create(string fileName) override{
	   string folderName;
      	   //string fileName;	
           cout<<"Enter the name of the Database to be opened: "<<endl;
           getline(cin, folderName);
           string databasePath = dbpath + folderName;
           //ofstream file(tablePath, ios::app);
           
           if(!exists(databasePath)){
            cerr<<"Database does not exists."<<endl;
           } else{
           	   ofstream folderName(databasePath);
           	   cout<<"Eneter Table to be created: "<<endl;
			   getline(cin, fileName);
			   //createpath = dbpath + folderName+"/"+fileName;
			   string createpath = databasePath +"/" + fileName;
			   if(!exists(createpath))
			   {
			      ofstream file(createpath, ios::app);
			      cout<<"Table created successfully" << fileName << endl;
			   }
			   else {
			     cerr<<"Error: Already Table is exists.";
			    }
            }
       }
    
      void dbremove() override{
      	    string folderName;
      	    string fileName;
            cout<<"Enter the name of the Database to be opened: "<<endl;
	    getline(cin, folderName);
	    string databasePath = dbpath + folderName;
	    if(!exists(databasePath)){
		    cerr<<"Database does not exists."<<endl;
		   } else{
		   	   ofstream folderName(databasePath);
		   	    cout<<"Enter Table to delete:  "<<endl;
			    getline(cin, fileName);
			    string removeTablePath = databasePath + "/" + fileName;
		      		if(!exists(removeTablePath)){
		      		     cerr<<"Error: Table does not exists."<<endl;
		      		} else{
		      		   remove_all(removeTablePath);
		      		   cout<<"sucessfully deleted the file.";
		      		 }
		      }
          }
    
	void dblist(ofstream &folder) override{

	string folderPath = folderName + "/" + fileName;

	try {
		// Check if the folder exists
		if (!exists(folderPath) || !is_directory(folderPath)) {
		    std::cerr << "Data base is not exists\n";
		    return;
		}
	       
		bool foundCSV = false; // To check if we find any .csv files
	int count = 0;

		// Iterate through the directory and search for .csv files
		for (const auto& entry : directory_iterator(folderPath)) {
		    if (entry.path().extension() == ".csv") {
	++count;
			std::cout<< count << ". " << entry.path().stem().string() << std::endl;
			foundCSV = true;
		    }
		}
	if (!foundCSV) {
	    std::cout << "No table found in Database!\n";
	}
	   } catch (const filesystem_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return;
	    }  
	    };

     // void insertData() {}
         
      	  
      
      
      void readData() {
      	string folderName;
      	string fileName;
        cout << "Enter the name of the database: ";
        getline(cin, folderName);
        cout << "Enter the name of the table to read data from: ";
        getline(cin, fileName);

        string tablePath = dbpath + folderName + "/" + fileName;
        if (!exists(tablePath)) {
            cerr << "Error: Table does not exist." << endl;
            return;
        }

        ifstream file(tablePath);
        if (!file.is_open()) {
            cerr << "Error: Unable to open table." << endl;
            return;
        }

        string line;
        cout << "Contents of table " << fileName << ":" << endl;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
      }

      
      void updateData() {
      	string folderName;
      	string fileName;
        cout << "Enter the name of the database: ";
        getline(cin, folderName);
        cout << "Enter the name of the table to update data in: ";
        getline(cin, fileName);

        string tablePath = dbpath + folderName + "/" + fileName;
        if (!exists(tablePath)) {
            cerr << "Error: Table does not exist." << endl;
            return;
        }

        ifstream fileIn(tablePath);
        if (!fileIn.is_open()) {
            cerr << "Error: Unable to open table." << endl;
            return;
        }

        vector<string> lines;
        string line;
        while (getline(fileIn, line)) {
            lines.push_back(line);
        }
        fileIn.close();

        cout << "Enter the line number to update (starting from 1): ";
        int lineNumber;
        cin >> lineNumber;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (lineNumber < 1 || lineNumber > lines.size()) {
            cerr << "Error: Invalid line number." << endl;
            return;
        }

        cout << "Enter the new data: ";
        string newData;
        getline(cin, newData);

        lines[lineNumber - 1] = newData;

        ofstream fileOut(tablePath);
        for (const auto &l : lines) {
            fileOut << l << endl;
        }
        fileOut.close();

        cout << "Data updated successfully." << endl;
    };
      
      //void loadData() {}
      
      //void saveData() {}

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

int main()
{
   string param;	
   //Database dbManager(param);
   DBStructure* dbManager = new Database();
   
    string command;

    cout << "Welcome to the DBMS system! Type your commands below:\n";

    while (true) {
        cout << "\n> ";
        getline(cin, command);

        stringstream ss(command);
        string action, param;
        ss >> action;

        if (action == "CREATE") {
            ss >> action; // Next part of command
            if (action == "DATABASE") {
                ss >> param;
                if (!param.empty()) {
                    dbManager->create(param);
                } else {
                    cout << "Error: No database name provided.\n";
                }
}
}
}
	return 0;	
}
