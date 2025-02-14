
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem> // For listing files
#include <sstream>
#include <regex>

using namespace std;
namespace fs = std::filesystem;

// Function to display the menu
void displayMenu() {
}

// Function to validate input using a regex
bool validateInput(const string& value, const string& type) {
    return false;
}

// Function to create a new CSV file
void createDatabase() {
}

// Function to list the databases (CSV files in the current directory)
void listDatabase() {
}

// Function to update an existing CSV file
void updateDatabase() {
}

// Function to insert data into an existing CSV file
void insertData() {
}

int main() {
    int choice;

    do {
        displayMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                createDatabase();
                break;
            case 2:
                listDatabase();
                break;
            case 3:
                updateDatabase();
                break;
            case 4:
                insertData();
                break;
            case 5:
                cout << "Exiting the program. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }

    } while (choice != 5);

    return 0;
}
	

