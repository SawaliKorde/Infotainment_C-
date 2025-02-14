/* Parse a CSV file and output a JSON file with same data

Understanding the problem and how it works
So an input .csv file after processing should be converted to .json file but retain same data
-----------------------------------------------------------------------------------------------
Class and Objects
we will have the input file as an object
Object - File
Class - FileData
- reads and validates the csv file

Child class - FileParser
- contains method to parse the csv file to json file
-------------------------------------------------------------------------------------------------
Definition of Class FileData
it will take file input and validate if csv file or not
Definition of Class FileParser inherits from File data
it will only have one method which is parse_file() which we will invoke to parse the csv to json.

----------------------------------------------------------------------------------------------
we will be using jsoncons which is a c++ header-only library for working and parsing json data
the jsoncons library already has a built in funtion named
 -  csv_reader - to read the csv file
 - decode_csv - convert the csv data to json object
 - json_output.to_string - convert json object to string again


-----------------------------------------------------------------------------------------------
Pseudo code implementation
1. In class FileData
1.1.1  Take csv file input from user
1.1.2 Read the csv file. (CSV file reading completed)
1.1 In class FileParser use getter and setter for file name
1.2  Validate if proper csv file or not. If not return error - "not a valid csv file!
2. By using built-in function of jsoncons library in method file_parser
	2.1 Using the decode_csv buit-in function convert the csv to json object (Conversion to json object completed)
	3.3 Using built-in function json_output - convert the json object to string again
	3.4 Create the json file using outfile

------------------------------------------------------------------------------------------------
Code implementation*/

//Including necessary header files for file operation and parsing the file
//Using Jsoncons library for parsing csv file
//Download the jsoncons library by using git clone https://github.com/danielaparker/jsoncons.git
//Setup correct path for usage of json.hpp and csv.hpp

#include<iostream>
#include<fstream>
#include<string>
#include<jsoncons/json.hpp>
#include<jsoncons_ext/csv/csv.hpp>

using namespace std;
using namespace jsoncons::csv;
using namespace jsoncons;

class FileData{
	private:
	      string filename;
	      
	// Class Contructor
	public:
		FileData(string fname): filename(fname) {}
	
	void set_file_name(string fname){
		filename = fname;
	}
	
	string get_file_name(){
		return filename;
	}
	
	void check_if_valid_csv(){
		// logic to validate csv file or not
		ifstream ifile(filename);
		if(!ifile.is_open()){
			cerr << " Unable to open file" << "\n";
			
		}
		string extension = filename.substr(filename.find_last_of(".") + 1);
		if(extension != "csv"){
			cerr << "File is not a CSV. Wrong input";
		}
			
		ifile.close();
		
		}	
	};



class FileParser: public FileData {
	public:
		FileParser(string fname):FileData(fname){}
	
	void parse_csv_to_json(){
		string filename = get_file_name();
		ifstream infile(filename);
		
		if(infile.is_open()){
			cout << "File opened successfully" << "\n";
		}
		
		csv::csv_options options;
		options.assume_header(true).trim(true).ignore_empty_values(true).column_types("string,string,string,string");
		
		
		ojson tasks = csv::decode_csv<ojson>(infile, options);
		size_t pos = filename.rfind(".csv");
		string json_filename = (pos != string::npos) ? filename.replace(pos,4,".json"): filename + ".json";
		ofstream out_file(json_filename);
		out_file << jsoncons::pretty_print(tasks) << "\n";
		out_file.close();
		cout << "Your Json file is ready" << "\n";
		
		
	}
};

int main(){
	FileParser file("/home/rps/Documents/data.csv");
	file.check_if_valid_csv();
	file.parse_csv_to_json();
	return 0;
}



