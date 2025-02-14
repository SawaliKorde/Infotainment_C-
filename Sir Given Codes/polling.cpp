#include<iostream>
#include<string>
#include<fstream>
#include<chrono>
#include<thread>
#include<sys/stat.h>

using namespace std;

class File
{
	private:
		string filename;
	public:
		File(string file) : filename(file){}
	void writeToFile()
	{
		// Open the file in append mode for writing
		ofstream outfile(filename, ios:: app);
	if(!outfile.is_open())
	{
		cerr << "Error: Unable to open file for writing! " << "\n";
		return ;
	}
	else
	{
		outfile << " New content is added in the file:" << "\n";
		outfile.flush();
	}
	outfile.close();
	}

	// read method

	string readFromFile()
	{
		string line;
		ifstream infile(filename);
	if(!infile.is_open())
		{
			cerr << "Fail to open" ;
		}
	else
		{
		getline(infile, line);
		cout << "Continue reading" << line << "\n";
		infile.close();
		}
	return line;
	}
};

class Polling
{
	private:
		File& file; // Referencing "File" class
		string lastContent;
		int WriteCounter = 0;
		string content;
	public:
		Polling(File& f) : file(f), lastContent(""){} // constructor
		void filepoll()
		{
			while (true)
	{
		if (WriteCounter % 5 == 0)
		{
			file.writeToFile();
			cout << "Writing new data at iteration:" << WriteCounter << "\n";
			content += file.readFromFile(); // If the content is changed read and print
			// WriteCounter++; // Update the Tracker
		}
			if (lastContent != content)
			{
	    			content = lastContent;
				//cout << "\n" << "Change in content" << content;
			}
		// Sleep for 1 second before going again
        	this_thread::sleep_for(chrono::seconds(1));
       	 	WriteCounter++;
       	 	}

	}
};



int main()
{
//const string filename("ABC.txt");
File file("ABC.txt");
//file.writeToFile();
file.readFromFile();

Polling poll(file);
poll.filepoll();

return 0;
}


