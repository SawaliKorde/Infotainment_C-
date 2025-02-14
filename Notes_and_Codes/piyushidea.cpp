#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

// Function to display the menu
void displayMenu() {
    cout << "\nMenu:" << endl;
    cout << "1. Create Database" << endl;
    cout << "2. List Database" << endl;
    cout << "3. Exit" << endl;
    cout << "Enter your choice: ";
}

// Function to create a new CSV file
void createDatabase() {
    string filename;
    cout << "Enter the name of the CSV file to create (without extension): ";
    cin >> filename;
    filename += ".csv";

    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Failed to create the file." << endl;
        return;
    }

    int numAttributes;
    cout << "Enter the number of attributes: ";
    cin >> numAttributes;

    vector<string> attributes;
    vector<string> types;

    for (int i = 0; i < numAttributes; ++i) {
        string attribute, type;
        cout << "Enter the name of attribute " << i + 1 << ": ";
        cin >> attribute;
        cout << "Enter the type of attribute " << i + 1 << " (e.g., int, string): ";
        cin >> type;
        attributes.push_back(attribute);
        types.push_back(type);
    }

    // Write attributes to the file
    for (size_t i = 0; i < attributes.size(); ++i) {
        file << attributes[i];
        if (i < attributes.size() - 1) file << ",";
    }
    file << "\n";

    // Optionally write types as a second row
    for (size_t i = 0; i < types.size(); ++i) {
        file << types[i];
        if (i < types.size() - 1) file << ",";
    }
    file << "\n";

    file.close();
    cout << "Database created successfully as " << filename << endl;
}

// Function to list the databases (CSV files in the current directory)
void listDatabase() {
    // Placeholder for listing files (platform-specific code needed for full implementation)
    cout << "Listing database feature is not implemented in this example." << endl;
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
                cout << "Exiting the program. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }

    } while (choice != 3);

    return 0;
}
