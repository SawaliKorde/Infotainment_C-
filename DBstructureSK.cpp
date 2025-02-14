#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <set>
#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <chrono>


using namespace std;
namespace fs = std::filesystem;

string dbpath = "/home/rps/Documents/"; //global

struct Record {
    map<string, string> columns;
};
class DatabaseOperations {
protected:
    mutex& operationMutex;

public:
    DatabaseOperations(mutex& mtx) : operationMutex(mtx) {}
    virtual ~DatabaseOperations() = default;

    virtual void create(const string& name) = 0;
    virtual void delete_op(const string& name) = 0;
    virtual void modify(const string& name) = 0;
    
    bool performFileOperation(const function<bool()>& operation) {
        lock_guard<mutex> lock(operationMutex);
        try {
            return operation();
        } catch (const exception& e) {
            cout << "Operation failed: " << e.what() << endl;
            return false;
        }
    }
};

class Table: public DatabaseOperations {
    private:
    string name;
    vector<string> column_names;
    vector<Record> rows;

public:
     // Default constructor needed for map
    Table() : DatabaseOperations(*new mutex()) {}

    // Constructor with mutex
    Table(mutex& mtx) : DatabaseOperations(mtx) {}

    // Copy constructor
    Table(const Table& other) : DatabaseOperations(*new mutex()) {
        name = other.name;
        column_names = other.column_names;
        rows = other.rows;
        }
    
    // Getters and setters for private members
    void setName(const string& tableName) { name = tableName; }
    string getName() const { return name; }
    void setColumnNames(const vector<string>& cols) { column_names = cols; }
    vector<string> getColumnNames() const { return column_names; }

    void create(const string& name) override {
        performFileOperation([&]() {
            this->name = name;
            return true;
        });
    }

    void delete_op(const string& name) override {
        // Implementation for delete
        performFileOperation([&]() {
            // Your delete logic
            return true;
        });
    }

    void modify(const string& name) override {
        // Implementation for modify
        performFileOperation([&]() {
            // Your modify logic
            return true;
        });
    }

    void insert(vector<string> values) {
        performFileOperation([&]() {
            if (values.size() != column_names.size()) {
                cout << "Error: The number of values does not match the number of columns." << endl;
                return false;
            }

            Record r;
            for (size_t i = 0; i < values.size(); i++) {
                r.columns[column_names[i]] = values[i];
            }
            rows.push_back(r);
            cout << "Record inserted successfully." << endl;
            save_to_csv();
            return true;
        });
    }

    void select() {
        performFileOperation([&]() {
            if (rows.empty()) {
                cout << "No records found." << endl;
                return false;
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
            return true;
        });
    }


    void update(string column, string old_value, string new_value) {
        performFileOperation([&]() {
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
        });
    }

    

    /*static void join(const Table& table1, const Table& table2, const string& column_name) {
        performFileOperation([&]() {
        bool column_found = false;
        for (const string& col1 : table1.column_names) {
            if (col1 == column_name) {
                column_found = true;
                break;
            }
        }

        if (!column_found) {
            cout << "Column " << column_name << " not found in table1." << endl;
            return;
        }

        column_found = false;
        for (const string& col2 : table2.column_names) {
            if (col2 == column_name) {
                column_found = true;
                break;
            }
        }

        if (!column_found) {
            cout << "Column " << column_name << " not found in table2." << endl;
            return;
        }

        vector<string> all_columns = table1.column_names;
        all_columns.insert(all_columns.end(), table2.column_names.begin(), table2.column_names.end());

        for (const auto& row1 : table1.rows) {
            for (const auto& row2 : table2.rows) {
                if (row1.columns.at(column_name) == row2.columns.at(column_name)) {
                    for (const auto& col1 : table1.column_names) {
                        cout << row1.columns.at(col1) << "\t";
                    }
                    for (const auto& col2 : table2.column_names) {
                        cout << row2.columns.at(col2) << "\t";
                    }
                    cout << endl;
                }
            }
        }
        });
    }*/

    void save_to_csv() {
        performFileOperation([&]() {
        string folder_path = dbpath + name + "/";
        if (!fs::exists(folder_path)) {
            fs::create_directory(folder_path);
        }

        ofstream file( folder_path + name +  ".csv");

        for (size_t i = 0; i < column_names.size(); i++) {
            file << column_names[i];
            if (i != column_names.size() - 1) {
                file << ",";
            }
        }
        file << endl;

        for (auto& row : rows) {
            for (size_t i = 0; i < column_names.size(); i++) {
                file << row.columns.at(column_names[i]);
                if (i != column_names.size() - 1) {
                    file << ",";
                }
            }
            file << endl;
        }
        file.close();
    });
    
};

class Database: public DatabaseOperations {

private:
     mutex dbMutex;
public:
    map<string, Table> tables;
    Database() : DatabaseOperations(dbMutex) {}

    void create_table(string name, vector<string> column_names, string db_name) {
        performFileOperation([&]() {
            Table table(dbMutex);
            table.setName(name);
            table.setColumnNames(column_names);

            string table_file = dbpath + db_name + "/" + name + ".csv";
            ofstream file(table_file);
            if (!file) {
                cout << "Failed to create table file: " << table_file << endl;
                return false;
            }

            for (size_t i = 0; i < column_names.size(); i++) {
                file << column_names[i];
                if (i < column_names.size() - 1) {
                    file << ",";
                }
            }
            file << endl;
            file.close();

            tables[name] = move(table);
            cout << "Table " << name << " created successfully." << endl;
            return true;
        });
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
    
    

public:
    ThreadPool threadpool();
    DBMS() : DatabaseOperations(fileMutex), threadpool(4) {} 

    void create_database(string name) {
    performFileOperation([&]() {
    try {
        string full_path = dbpath + name;  // Construct the full path
        cout << "Attempting to create directory at: " << full_path << endl;  // Debug line

        if (!fs::exists(full_path)) {
            if (fs::create_directory(full_path)) {
                cout << "Folder for database " << name << " created successfully." << endl;
                Database db;
                databases[name] = std::move(db);
                cout << "Database " << name << " created successfully." << endl;
            } else {
                cout << "Failed to create folder for database " << name << endl;
            }
        } else {
            cout << "Folder already exists for database " << name << endl;
            Database db;
            databases[name] = std::move(db);
        }
    } catch (const fs::filesystem_error& e) {
        cout << "Filesystem error: " << e.what() << endl;
    }
  });
  }
    void use_database(string name) {
         performFileOperation([&]() {
        if (databases.find(name) != databases.end()) {
            current_db = &databases[name];
            cout << "Switched to database " << name << "." << endl;
        } else {
            cout << "Database not found!" << endl;
        }
    });

    Database* get_current_db() {
        return current_db;
    }
   }

    void show_databases() {
        performFileOperation([&]() {
	fs::path currentDir = dbpath;
	int count = 0;

	for (const auto& entry : fs::directory_iterator(currentDir)) {
	   if (entry.is_directory() && entry.path().filename().string()[0] != '.')
	{
	++count;
	       cout<< count << ". " << entry.path().filename().string() << endl;
	   }
        }
     });
  }
 

    void run_command(const string& cmd) {
        ThreadPool.enqueue([this,cmd](){
        performFileOperation([&]() {
        stringstream ss(cmd);
        string command;
        ss >> command;

        if (command == "create") {
            string obj_type;
            ss >> obj_type;

            if (obj_type == "database") {
                string db_name;
                ss >> db_name;
                create_database(db_name);
                
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
    current_db->create_table(table_name, columns, current_db_name);
} else {
    cout << "No database selected." << endl;
}
            }
        }
        else if (command == "use") {
            string db_name;
            ss >> db_name;
            use_database(db_name);
        }
        else if (command == "show") {
            string obj_type;
            ss >> obj_type;

            if (obj_type == "databases") {
                show_databases();
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
                
                if (values.size() != table->getColumnNames().size()) {
                cout << "Error: Number of values doesn't match the number of columns." << endl;
                }
                else {
                    table->insert(values);
                }
            }
        }
        else if (command == "select") {
            string table_name;
            ss >> table_name;

            Table* table = current_db ? current_db->get_table(table_name) : nullptr;
            if (table) {
                table->select();
            }
        }
        else if (command == "join") {
            string table1_name, table2_name, column_name;
            ss >> table1_name >> table2_name >> column_name;

            Table* table1 = current_db ? current_db->get_table(table1_name) : nullptr;
            Table* table2 = current_db ? current_db->get_table(table2_name) : nullptr;

            if (table1 && table2) {
                Table::join(*table1, *table2, column_name);
            }
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
    });
   });
   
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

int main() {
    DBMS dbms;
    dbms.interactive_shell();
    return 0;
}
