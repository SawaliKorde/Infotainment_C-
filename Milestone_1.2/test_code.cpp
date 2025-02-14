#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>

using namespace std;
using namespace std::filesystem;

string dbpath = "/home/rps/Documents/";

int main() {
    string folderName;
    string fileName;
    string createpath;         
    cout<<"Enter the name of the Database.\n"; 
    getline(cin, folderName);
    cout<<"basepath:"<<dbpath<<endl;
    string folderpath = dbpath + folderName;
    cout<<"folder path:\n"<<folderpath<<endl;
   
    if(!exists(folderpath)){
    	cerr<<"Database does not exists."<<endl;
   }else{
   cout<<"Enter the name of the Table to create:\n";
   getline(cin, fileName);
   createpath = dbpath + folderName + "/" + fileName;
   //ofstream folder(folderName);
   cout<<createpath<<endl;
   if(!exists(createpath)){
      ofstream file(createpath, ios::app);
      cout<<"Table created successfully \n" << fileName << endl;
   }else {
     cerr<<"Error: Table Already Exists.\n";
    }
}

    return 0;
}

