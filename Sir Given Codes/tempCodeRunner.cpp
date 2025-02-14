#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<chrono>
#include<ctime>

using namespace std;

class Device{
	private:
		int id;
		string location;
	public:
		Device(int did, string dlocation):id(did),location(dlocation){}
		
		int getId(){
			return id;
		}
		
		string getLocation(){
			return location;
		}
		
		void setId(int d_id){
			id = d_id;
		}
		
		void setLocation(string d_location){
			location = d_location;
		}	
	
	void sendData(){};
	void getTime(){
		
		auto now = chrono::system_clock::now();
		time_t currentTime = chrono::system_clock::to_time_t(now);
		cout << ctime(&currentTime) << "Hello ";	
	}
	
	
};

class TempSensor :public Device{
	private:
	vector<double> temperature;
	vector<bool> rain_possibility;
	public:
	
	TempSensor(int did, string dlocation,double temp, bool rain):Device(did,dlocation),temperature(temp),rain_possibility(rain){}
	
	vector<double> getTemperature(){
		return temperature;
		}
	vector<bool> getRainPossibility(){
		return rain_possibility;
		}
	
	void setTemperature(double temp){
		temperature.push_back(temp);
		}
	void setRainPossibility(bool rp){
		rain_possibility.push_back(rp);
		}	
	
	void sendData() override{
		
		
	
	}
};

/*class DataCollector {
		private:
		vector<Device*> devices;
		bool collecting=false;
		public:
		string filename;
			
	DataCollector(vector<Device*> otherDevices,bool collect,string fname) : devices(otherDevices),collecting(collect),filename(fname) {}
	
	void addDevice(Device* device){
		devices.push_back(device);
	}
	
	void startCollecting(){
		collecting = true;
		for(Device* device:devices){
			device->sendData();
		}
	}
	
	void stopCollecting(){
		collecting=false;
	}
	void pauseCollecting(){}
	
	void displayLiveData(){
			
	}
	void writeDataToFile(const vector<Device *> devices,string fname){
		ofstream outfile(filename,ios::app);
		if(!outfile){
			cerr<<"Error can not open the file for writing \n";
			return;
		}
		for(const auto& device:devices){
			outfile<<device<<"\n";
		}
		outfile.close();
		cout<<"Data is written to the file \n";
	}
	~DataCollector(){
		for(Device* device:devices){
			delete device;
		}
	}	
	
};*/

int main(){
	Device device(1,"abc");
	device.getTime();
	//TempSensor tempSensor1(1,"abc",40,false);
	//tempSensor1->addTemperature(40.0);
	//DataCollector datacollector1(&tempSensor1,false,"myfile.txt");
	//datacollector1.addDevice(&tempSensor1);
	//datacollector1.writeDataToFile(&tempSensor1,"myfile.txt");
	}
