#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<chrono>
#include<ctime>
#include<sstream>

using namespace std;

class Device{
	private:
		int id;
		string location;
	public:
		Device() {};
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
	
	virtual void sendData(){};
	time_t getTime(){
		
		auto now = chrono::system_clock::now();
		return  chrono::system_clock::to_time_t(now);
		//cout << ctime(&currentTime) << "Hello ";
			
	}
	
	virtual ~Device(){};
	
	
};

class TempSensor :public Device{
	private:
	vector<double> temperature;
	vector<bool> rain_possibility;
	
	public:	
	TempSensor() {};
	TempSensor(int did, string dlocation):Device(did,dlocation){}
		

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
		cout << Device::getId() <<  Device::getLocation();
		for(const auto& temp:temperature){
			cout <<  temp;

		}
		for(const auto& rain:rain_possibility){
			cout << rain;

		}		
				
			
	}
};



class PressureSensor :public Device{
	private:
		vector<double> pressure;
	
	public:	
		PressureSensor() {};
		PressureSensor(int did, string dlocation):Device(did,dlocation){}
		
	
	
	vector<double> getPressure()
	{
		return pressure;
	}
	
	void setPressure(double p)
	{
		pressure.push_back(p);
	}
		
	
	void sendData() override
	{
		cout << Device::getId() <<  Device::getLocation();
		for(const auto& p:pressure){
			cout <<  p;

		}		
		
	
	}
};


class DataCollector {
		private:
		vector<Device*> devices;
		bool collecting=false;
		public:
		string filename;
		
		DataCollector(string fname) : filename(fname) {}
	 
	 
	
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
	void writeDataToFile(){
		ofstream outfile(filename,ios::app);
		if(!outfile){
			cerr<<"Error can not open the file for writing \n";
			return;
		}
		
		for(const auto &device:devices){
			TempSensor* tempsensor = dynamic_cast<TempSensor*>(device) ;
			PressureSensor* pressuresensor = dynamic_cast<PressureSensor*>(device) ;
			time_t dtime = device->getTime();
			if(tempsensor){
				for(int i=0;i<=5;++i){
				
					cout << device->getId() << " ";
					cout << device->getLocation()  << " ";
					cout << ctime(&dtime)  << " ";
					cout << (tempsensor->getTemperature().empty()? 0 : tempsensor->getTemperature().back()+i*0.5)  << " ";
					cout << (tempsensor->getRainPossibility().empty() ? false : tempsensor ->  getRainPossibility().back())  << " ";
				
					//outfile << ""
					outfile << "Device Id:" << device->getId()  << " ";
					outfile << device->getLocation()  << " ";
	outfile << "Temperature:" << (tempsensor->getTemperature().empty()? 0 : tempsensor->getTemperature().back()+i*0.5)  << " ";
	outfile << (tempsensor->getRainPossibility().empty() ? false : tempsensor->getRainPossibility().back())  << " ";
					outfile << ctime(&dtime)  << " ";
				}
				
				
			}
			
			if(pressuresensor)
			{
				for(int i=0;i<=5;++i){
				
					cout << device->getId() << " ";
					cout << device->getLocation()  << " ";
					cout << ctime(&dtime)  << " ";
					cout << (pressuresensor->getPressure().empty()? 0 : pressuresensor->getPressure().back()+i*0.6)  << " ";
									
				
					outfile << "Device Id:" << device->getId()  << " ";
					outfile << device->getLocation()  << " ";
					
					outfile << "Pressure:" << (pressuresensor->getPressure().empty()? 0 : pressuresensor-> getPressure().back()+i*0.6)  << " ";
					
					outfile << ctime(&dtime)  << " ";
				}
			}
			
			
		}
		outfile.close();
		cout<<"Data is written to the file,outof write file \n"; 
	}
	~DataCollector(){
		for(Device* device:devices){
			delete device;
		}
	}	
	
};

class DataAnalyzer{
	private :
		string fileName;
		vector<double> temperature;
		vector<double> pressure;
		vector<bool> rainPossibility;
		
	public:
		DataAnalyzer(string fname) : fileName(fname) {}
		
	void readFile(){
		ifstream infile(fileName,ios::in);
		if(!infile){
			cerr << "File cannot open for reading \n";
			return;
		}
		string line;
		int linecount = 0;
		double totaltemp=0;
		double totalPressure = 0;
		double maxtemp = 0;
		double maxPressure = 0;
		double mintemp = 0;
		double minPressure = 0;
		int countt = 0;
		int countp = 0;
		
		cout << "Content in the file \n";
		
		while(getline(infile,line)){
			linecount++;
			stringstream ss(line);
			string tempstr,rainstr;	// This will hold the rain and temperature values from the line in the file
			
			// The data comes in as string. We will need to convert it into a number for computing
			// Each line has Device Id, Temperature, rain,timestamp
			
			double temp;
			double pressure;
			bool rain;
			string date;
			
			// Read the line, parse (Make sense of what is come in and get what we want exactly)
			
			if(line.find("Temperature:") != string::npos) 
			{
						//cout << tempstr <<  "\n";
						countt++;
						size_t pos = line.find("Temperature:");
						string sub = line.substr(pos + 12);
						stringstream ss_temp(sub);
						ss_temp >> temp;
						//cout << temp  << "\n";
						//cout << sub  << "\n";
						//ss >> tempstr >> temp >> rainstr >> rain;
						//Use cout to see these variables -For you to do
						//cout << temp <<  "\n";
						totaltemp+=temp;
				
						if (temp > maxtemp) maxtemp=temp;
						if (temp < mintemp) mintemp = temp;
			
			}
			
			if(line.find("Pressure:") != string::npos) 
			{
						countp++;
						size_t pos = line.find("Pressure:");
						string sub = line.substr(pos + 9);
						stringstream ss_pressure(sub);
						ss_pressure >> pressure;
						//ss >> pressStr >>pressure;
						
						
						totalPressure+=pressure;
				
						if (pressure > maxPressure) maxPressure=pressure;
						if (pressure < minPressure) minPressure = pressure;
			
			}

		}
		
		// Compute the average temperature
		cout << "Calculating Average Temperature\n ";
		double avgtem = totaltemp /countt ;
		cout << "Line Count : " << linecount << "\n";
		cout << "Total Temperature  : " << totaltemp  << "\n";
		cout << "Average Temperature : "<< avgtem << "\n";
		
		//compute the average pressure
		cout << "Calculating Average Pressure\n ";
		double avgPressure = totalPressure / countp ;
		cout << "Line Count : " << linecount << "\n";
		cout << "Total Pressure  : " << totalPressure  << "\n";
		cout << "Average Pressure : "<< avgPressure;
		
		infile.close();
		
	}
	
	
};

int main(){
	Device device(1,"abc");
	device.getTime();
	
	// Create an instance of each of the sensors that we want to collect data for
	TempSensor *tempSensor1 = new TempSensor(1,"abc");
	tempSensor1->setTemperature(25);
	tempSensor1->setRainPossibility(true);
	tempSensor1->sendData();	// Keep sending data, like FM radio station - We will tune in using the Data Collector class
	
	//TempSensor tempSensor1(1,"abc",23.5,true);
	//TempSensor tempSensor2(2,"abc",23.5,true);

	// Sensor 2
	TempSensor *tempSensor2 = new TempSensor(2,"defc");
	tempSensor2->setTemperature(26);
	tempSensor2->setRainPossibility(true);
	tempSensor2->sendData();
	
	
	//pressuresensor1
	PressureSensor *pressureSensor1 = new PressureSensor(3,"xyz");
	pressureSensor1->setPressure(50);
	pressureSensor1->sendData();
	
	//DataCollector dataCollector(&tempSensor1,true,"myfile.txt");
	DataCollector dataCollector("myfile.txt");

	// Add the devices that we need to monitor, so that they work on their own
	dataCollector.addDevice(tempSensor1);
	dataCollector.addDevice(tempSensor2);
	dataCollector.addDevice(pressureSensor1);

	// Trigger the data collection
	dataCollector.startCollecting();
	// Write the collected data to the file/target
	dataCollector.writeDataToFile();
	cout << "Back to main \n" ;
	
	DataAnalyzer dataAnalyzer("myfile.txt");
	dataAnalyzer.readFile();
	
	
	
	
	
	
	
	
	
	//tempSensor1->addTemperature(40.0);
	//DataCollector datacollector1(&tempSensor1,false,"myfile.txt");
	//datacollector1.addDevice(&tempSensor1);
	//datacollector1.writeDataToFile(&tempSensor1,"myfile.txt");
	}
