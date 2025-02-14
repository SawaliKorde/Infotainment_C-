#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <set>
#include <fstream>
#include <filesystem>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <atomic>


using namespace std;
namespace fs = std::filesystem;

string dbpath = "/home/rps/Documents/"; //global

mutex coutMutex;
    void logThreadStatus(const std::string &operation, const std::string &status) {
       std::lock_guard<std::mutex> guard(coutMutex); // Separate mutex for cout
       std::cout << "Thread [" << std::this_thread::get_id() << "] - " << operation << ": " << status << std::endl;
                        
       std::cout.flush();
    }
 
//abstract class   
class DataOperation {
protected:
    mutex& fileMutex;
    
public:
    DataOperation(mutex& mutex) : fileMutex(mutex) {}
    
    virtual void create(const string& name) = 0;
    virtual void select(const string& name) = 0;
    virtual void update(const string& name) = 0;
    virtual void remove(const string& name) = 0;
    virtual void show() = 0;
    virtual void save() = 0;
    virtual void load() = 0;
    
    virtual ~DataOperation() = default;
};
struct Record {
    map<string, string> columns;
};


struct Table: public DataOperation {
    string name;
    vector<string> column_names;
    vector<Record> rows;

   

    void insert(vector<string> values, mutex &fileMutex) {
        logThreadStatus("Insert", "Started");
	lock_guard<mutex> lock(fileMutex);
        if (values.size() != column_names.size()) {
            cout << "Error: The number of values does not match the number of columns." << endl;
            return;
        }

        Record r;
        for (size_t i = 0; i < values.size(); i++) {
            r.columns[column_names[i]] = values[i];
        }
        rows.push_back(r);
        cout << "Record inserted successfully." << endl;
        save_to_csv(fileMutex);
    }

    void select(mutex &fileMutex) {
        logThreadStatus("Selection", "Started");
	lock_guard<mutex> lock(fileMutex);
        if (rows.empty()) {
            cout << "No records found." << endl;
            return;
        }
        for (auto& col : column_names) {
            cout << col << "\t";
        }
        cout << endl;
        for (auto& row : rows) {
            for (auto& col : column_names) {
                cout << row.columns.at(col) << "\t";
            }
            cout << endl;
        }
    }

    void update(string column, string old_value, string new_value, mutex &fileMutex) {
        logThreadStatus("Update", "Started");
	lock_guard<mutex> lock(fileMutex);
        bool updated = false;
        for (auto& row : rows) {
            if (row.columns[column] == old_value) {
                row.columns[column] = new_value;
                updated = true;
            }
        }
        if (updated)
            cout << "Update successful." << endl;
        else
            cout << "No matching records found." << endl;
    }

    void delete_record(string column, string value, mutex &fileMutex) {
        logThreadStatus("Delete", "Started");
	lock_guard<mutex> lock(fileMutex);
        auto it = remove_if(rows.begin(), rows.end(), [&column, &value](Record& r) {
            return r.columns[column] == value;
        });
        if (it != rows.end()) {
            rows.erase(it, rows.end());
            cout << "Record(s) deleted." << endl;
        } else {
            cout << "No matching records found." << endl;
        }
    }
    
    void save_to_csv(map<int, Table> tables, promise<void> savepromise, mutex &fileMutex) {
    	 logThreadStatus("Save", "Started");
	 lock_guard<mutex> lock(fileMutex);
    	 try{
    	    this_thread::sleep_for::(std::chrono::seconds(5));
    	    
	 string savefilepath = dbpath + name + "/";
	 ofstream file(savefilepath);
		    if (!file.is_open()) {
		        throw runtime_error("Failed to open file for writing: " + savefilepath);
		    }
	 for(const auto& entry: tables){
	      file<<entry.first<<","<<entry.second<<"\n";
	   }
	   savepromise.set_value();
	   cout<<"Data saved to file"<<endl;
	 }catch{
	      (const exception& e) {
		cerr << "Error saving to CSV: " << e.what() << endl;
	  }
	}
};
}

class Database:  public DataOperation  {
public:
    Database(mutex& mutex) : DataOperation(mutex) {}
    map<string, Table> tables;
    
    void create(const string& name) override {
        create_table(name, vector<string>(), "db_name", fileMutex);
    }

    void create_table(string name, vector<string> column_names, string db_name, mutex &fileMutex) {
	    logThreadStatus("create table", "Started");
	    lock_guard<mutex> lock(fileMutex);
	    Table table;
	    table.name = name;
	    table.column_names = column_names;

	    try {
		// Create and initialize the table file
		string table_file = dbpath + db_name +"/" + name + ".csv";
		cout << table_file << "file path" << endl;
		ofstream file(table_file);
		if (!file) {
		    cout << "Failed to create table file: " << table_file << endl;
		    return;
		}

		// Write column headers to the CSV file
		for (size_t i = 0; i < column_names.size(); i++) {
		    file << column_names[i];
		    if (i < column_names.size() - 1) {
		        file << ",";
		    }
		}
		file << endl;
		file.close();

		// Store table in memory
		tables[name] = table;
		cout << "Table " << name << " created successfully." << endl;

	    } catch (const exception& e) {
		cout << "Error creating table: " << e.what() << endl;
	    }
	}

	    Table* get_table(string name) {
		if (tables.find(name) != tables.end())
		    return &tables[name];
		else {
		    cout << "Table not found!" << endl;
		    return nullptr;
		}
	    }
};

class DBMS {
private:
    map<string, Database> databases;
    Database* current_db;
    mutex fileMutex;
    ThreadPool pool;

public:
    DBMS() : current_db(nullptr), pool(4) {}
   
    void load(mutex &fileMutex) {
        logThreadStatus("Load", "Started");
	lock_guard<mutex> lock(fileMutex);
	    try {
		for (const auto& db_entry : fs::directory_iterator(dbpath)) {
		    if (db_entry.is_directory()) {
		        string db_name = db_entry.path().filename().string();
		        Database db;

		        for (const auto& table_entry : fs::directory_iterator(db_entry.path())) {
		            if (table_entry.path().extension() == ".csv") {
		                string table_name = table_entry.path().stem().string();
		                Table table;
		                table.name = table_name;

		                ifstream file(table_entry.path());
		                if (!file) {
		                    cout << "Failed to open file: " << table_entry.path() << endl;
		                    continue;
		                }

		                string line;
		                if (getline(file, line)) {
		                    stringstream ss(line);
		                    string col;
		                    while (getline(ss, col, ',')) {
		                        table.column_names.push_back(col);
		                    }
		                }

		                while (getline(file, line)) {
		                    stringstream ss(line);
		                    string value;
		                    Record record;

		                    for (const string& col : table.column_names) {
		                        if (getline(ss, value, ',')) {
		                            record.columns[col] = value;
		                        }
		                    }
		                    table.rows.push_back(record);
		                }

		                file.close();
		                db.tables[table_name] = table;
		            }
		        }

		        databases[db_name] = db;
		        //cout << "Database " << db_name << " loaded successfully." << endl;
		    }
		}
	    } catch (const fs::filesystem_error& e) {
		cout << "Filesystem error during load: " << e.what() << endl;
	    } catch (const exception& e) {
		cout << "Error during load: " << e.what() << endl;
	    }
	}

    void create_database(string name, mutex &fileMutex) {
        pool.enqueueAndWait([this, name, &fileMutex]() {
	    logThreadStatus("Create", "Started");
	    lock_guard<mutex> lock(fileMutex);
	    try {
		string full_path = dbpath + name;  // Construct the full path
		cout << "Attempting to create directory at: " << full_path << endl;  // Debug line

		if (!fs::exists(full_path)) {
		    if (fs::create_directory(full_path)) {
		        cout << "Folder for database " << name << " created successfully." << endl;
		        Database db;
		        databases[name] = db;
		        //cout << "Database " << name << " created successfully." << endl;
		    } else {
		        cout << "Failed to create folder for database " << name << endl;
		    }
		} else {
		    cout << "Folder already exists for database " << name << endl;
		    Database db;
		    databases[name] = db;
		}
	    } catch (const fs::filesystem_error& e) {
		cout << "Filesystem error: " << e.what() << endl;
	    }
        }
	}
    void use_database(string name, mutex &fileMutex) {
        logThreadStatus("Use of database", "Started");
	lock_guard<mutex> lock(fileMutex);
        if (databases.find(name) != databases.end()) {
            current_db = &databases[name];
            cout << "Switched to database " << name << "." << endl;
        } else {
            cout << "Database not found!" << endl;
        }
    }

    Database* get_current_db() {
        return current_db;
    }

    void show_databases(mutex &fileMutex) {
        logThreadStatus("Show", "Started");
	lock_guard<mutex> lock(fileMutex);
	fs::path currentDir = dbpath;
	int count = 0;

	for (const auto& entry : fs::directory_iterator(currentDir)) {
	  if (entry.is_directory() && entry.path().filename().string()[0] != '.')
	{
	++count;
	      cout<< count << ". " << entry.path().filename().string() << endl;
	  }
		}
	     }
 

    void run_command(const string& cmd) {
        stringstream ss(cmd);
        string command;
        ss >> command;

        if (command == "create") {
            string obj_type;
            ss >> obj_type;

            if (obj_type == "database") {
                string db_name;
                ss >> db_name;
                create_database(db_name, fileMutex);
               
            } else if (obj_type == "table") {
                string table_name;
                ss >> table_name;

                vector<string> columns;
                string column;
                while (ss >> column) {
                    columns.push_back(column);
                }

	if (current_db) {
	    // Find the name of the current database
	    string current_db_name;

	    for (const auto& database : databases) {
	if (&database.second == current_db) {
	   current_db_name = database.first;
	   break;
	}
	    }

	    // Use the database name in the function call
	    current_db->create_table(table_name, columns, current_db_name, fileMutex);
	} else {
	    cout << "No database selected." << endl;
	}
		    }
		}
		else if (command == "use") {
            string db_name;
            ss >> db_name;
            use_database(db_name, fileMutex);
        }
        else if (command == "show") {
            string obj_type;
            ss >> obj_type;

            if (obj_type == "databases") {
                show_databases(fileMutex);
            }
            else if (obj_type == "tables") {
                if (current_db) {
                    for (auto& table : current_db->tables) {
                   
                        cout << table.first << endl;
                    }
                } else {
                    cout << "No database selected." << endl;
                }
            }
        }
        else if (command == "insert") {
            string table_name;
            ss >> table_name;

            Table* table = current_db ? current_db->get_table(table_name) : nullptr;
            if (table) {
                vector<string> values;
                string value;
                while (ss >> value) {
                    values.push_back(value);
                }
               
                if (values.size() != table->column_names.size()) {
                    cout << "Error: Number of values doesn't match the number of columns." << endl;
                } else {
                    table->insert(values, fileMutex);
                }
            }
        }
        else if (command == "select") {
            string table_name;
            ss >> table_name;

            Table* table = current_db ? current_db->get_table(table_name) : nullptr;
            if (table) {
                table->select(fileMutex);
            }
        }
        else if (command == "join") {
            string table1_name, table2_name, column_name;
            ss >> table1_name >> table2_name >> column_name;

            Table* table1 = current_db ? current_db->get_table(table1_name) : nullptr;
            Table* table2 = current_db ? current_db->get_table(table2_name) : nullptr;

            /*if (table1 && table2) {
                Table::join(*table1, *table2, column_name);
            }*/
        }
        else {
            cout << "Unknown command!" << endl;
        }
    }

    void interactive_shell() {
        string input;
        while (true) {
            cout << "DBMS> ";
            getline(cin, input);
            if (input == "exit") {
                break;
            }
            run_command(input);
        }
    }
};

int main() {
    mutex fileMutex;
    //ThreadPool pool(4);
    promise<void> savepromise;
    future<void> savefuture = savepromise.get_future();
    thread saveThread(save_to_csv, tables, move(savepromise));
    try{
    	 cout<<"waiting for data to be saved."<<endl;
    	 savefuture.get();
    	 cout<<"save operation completed successfully"<<endl;
    	 }
    catch(const exception& e){
      cerr<<"Error:"<<e.what()<<endl;
     }
     savethread.join();
    	
    DBMS dbms;
    dbms.load(fileMutex);
    dbms.interactive_shell();
    dbms.pool.shutdown();
    dbms.pool.waitForCompletion();
    return 0;
}
