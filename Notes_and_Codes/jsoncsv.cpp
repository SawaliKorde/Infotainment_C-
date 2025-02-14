#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>


// Function to trim whitespace from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// Function to parse a JSON string and convert it to a vector of maps
std::vector<std::map<std::string, std::string>> parseJson(const std::string& jsonString) {
    std::vector<std::map<std::string, std::string>> jsonArray;
    std::map<std::string, std::string> jsonObject;
    std::istringstream jsonStream(jsonString);
    std::string line;

    while (getline(jsonStream, line)) {
        line = trim(line);
        if (line == "{" || line == "},") {
            if (!jsonObject.empty()) {
                jsonArray.push_back(jsonObject);
                jsonObject.clear();
            }
        } else if (line != "}" && line != "[") {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos && colonPos > 1 && colonPos < line.size() - 2) {
                std::string key = trim(line.substr(1, colonPos - 2));
                std::string value = trim(line.substr(colonPos + 1));
                if (!key.empty() && !value.empty()) {
                    if (value.back() == ',') value.pop_back();
                   if (value.front() == '"' && value.back() == '"') value = value.substr(1, value.size() - 2);
                    jsonObject[key] = value;
                }
            }
        }
    }
    if (!jsonObject.empty()) {
        jsonArray.push_back(jsonObject);
    }
    return jsonArray;
}

// Function to convert JSON data to CSV format
void jsonToCsv(const std::vector<std::map<std::string, std::string>>& jsonData, const std::string& csvFilePath) {
    std::ofstream csvFile(csvFilePath);

    std::cout << "Entering jsontocsv";

    if (!csvFile.is_open()) {
        std::cerr << "Failed to open CSV file." << std::endl;
        return;
    }

    // Write header
    if (!jsonData.empty()) {
    	std::cout << "entered to check json file empty or not" << "/n";
        for (const auto& pair : jsonData[0]) {
            csvFile << pair.first << ",";
        }
        csvFile.seekp(-1, std::ios_base::end); // Remove the last comma
        csvFile << "\n";
    }

    // Write data
    for (const auto& item : jsonData) {
        for (const auto& pair : item) {
            csvFile << pair.second << ",";
             
        }
        csvFile.seekp(-1, std::ios_base::end); // Remove the last comma
        csvFile << "\n";
    }

    csvFile.close();
}

int main() {
    std::string jsonFilePath = "data.json"; // Path to your JSON file
    std::string csvFilePath = "output.csv"; // Path to the output CSV file

    // Read the JSON file
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
    	
        std::cerr << "Failed to open JSON file." << std::endl;
        return 1;
    }

    std::string jsonString((std::istreambuf_iterator<char>(jsonFile)),
                            std::istreambuf_iterator<char>());
    jsonFile.close();

    // Parse JSON and convert to CSV
    auto jsonData = parseJson(jsonString);
    std::cout << jsonString << "json data" << "\n";
    jsonToCsv(jsonData, csvFilePath);

    std::cout << "Conversion complete. CSV saved to " << csvFilePath << std::endl;

    return 0;
}
