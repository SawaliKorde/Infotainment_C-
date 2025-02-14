/* what to do ?
	Have one file which is shared by two different processes.
	We use Threads
	Our App : STA (Sinlge Threaded Applcation) ==> Process
	APPlication
		|----Process1
		|    |----- t1 (function())
   		|    |----- t2
		|----Process2
		     |----- t3
		     |------t4 		
		----Process3

	What is a thread ?
		A thread is a process that runs on its own. 
		It is the smallest execution Unit 
		It will always run a function This is called as a thread start function

	Tell them to work in harmony :
		When 1 thread is running, others should wait
		When the thread is done running, it shoudl let go of the resource (file, socket...) so that others can write/read

	If two threads hold on to a resource (file,momory, socket...)
		One has to flag that it is using and the other has to wait
		Mutex, Sempaphores, pipes

		Operation		T1		T2		Tn
		Write			X		WT		WT
		Read			X		Write/WT	Write/WT

	Issue is sharing. Threads will flag when they are not to be disturbed (during write operations - write operation can change 
	 the meaning of the data, while a read operation can be retried.

	WHen writing or updating, Threads use a state called "ENTERING CRITICAL SECTION". This means all other threads are blocked from
	accessing the resource.	
	
	If two threads write at the same time - Deadlock - none is willing to let go of the resource. One has to sacrifice the data and retry 	again
		
	Terms like Thread SYNCHRONIZATION are all used to avoid Deadlocks, avoid resource contention.
	
	You can only start a thread, and wait for it to finish. WHy ? The OS has to schedule the Thread Execution based on the CPU
	availability.

	Two classes - Reader /Writer. We will create multiple threads and show the threads on the console as they read/write from a file.

*/

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>	// Mutually Exclusive - 1 writes -other waits
#include <vector>
#include <chrono>
#include <random>
#include <atomic>
#include <iomanip>

// Create an enumerator with statuses for the thread
enum status {IDLE, WRITE, READ, WAIT };


struct threadstatus {
			int threadno;
			status status;
			};

std::vector<threadstatus> mthreads(10);
std::mutex filemutex;
std::mutex statusmutex;
std::atmoic<bool> keeprunning(true);

void displaystatuses(){
	while(keeprunning) 
	{	
		std::cout << "\033[2J\033[H";
	
		//Display the header for the table 
		std::cout << std::setw(10) << "Thread No." 
			  << std::setw(10) << "Write"
	   		  << std::setw(10) << "Read"
	   		  << std::setw(10) << "Wait"
	   		  << std::setw(10) << "Idle" << std::endl;
		std::cout << std::string(50,"=") << std::endl;

	{
		std::unique_lock <std::mutex> lock(statusmutex);
		for(cosnt auto& status: mthreads)
		{
		std::cout << std::setw(10) << status.thread_no	
			  << std::setw(10) << (status.status == WRITE ? "\033[32mX\033[0m" : "")
			  << std::setw(10) << (status.status == READ  ? "\033[32mX\033[0m" : "")
			  << std::setw(10) << (status.status == WAIT  ? "\033[32mX\033[0m" : "")
			  << std::setw(10) << (status.status == IDLE  ? "\033[32mX\033[0m" : "")
			  << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

}
}

void threadfunction(int threadno) 
{
	std::default_random_engine generator;	//Use this for random values for later
	std::uniform_int_distribution<int> sleepdist(200,600);  // To make the thread sleep for random times
	std::ofstream file;

	//Enter the loop forever

	while(keeprunning) 
	{  
		//Let's assume and set the status to WAIT
		std::unique_lock<std::mutex> lock(statusmutex);
		mthreads[threadno].status =WAIT;

	

	std::this_thread::sleep_for(std::chrono::milliseconds(sleepdist(generator)));

	//Write to the file
	{
		std::unique_lock<std::mutex> lock(statusmutex);
		mthreads[threadno].status=WRITE;

	}

	{
		std::unique_lock<std::mutex> lock<filemutex>;
		file.open("tt.txt",std::ios::app);
		file << "Thread "<< threadno << "Writing at time " << std::time(nullptr) << "\n";
		file.close()
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(sleepdist(generator)));

	threads[threadno].status=READ;
	std::ifstream rf("tt.txt");
	std::string line;

	while(std::getline(rf,line)) 
			{
				std::cout << "read the file";
	
			}
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepdist(generator)));
		
		{
			std::unique_lock<std::mutex> lock(status_mutex);
			mthreads[threadno].status=IDLE;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(sleepdis(generator)));

	}
}



int main()
{
//Init thread statuses
for (int i =0;i<10;i++) 
{
	mthreads[i]={i,IDLE};
}

//Start threads
std::vector<std::thread> threads;
for (int i= 0;i<10;i++)
{
	threads.emplace_back(threadfunction,i);
}
//In this main thread, i want to display the status of each thread

	std::thread display_thread(displaystatuses);

//Keep running till the user ends the program by hitting any key
std::cout  << "Press any key to exit...." << std::endl;
std::cin.get();
keeprunning=false;

//Join the threads together !
display_thread.join();
for (auto& t:threads) 
	{
	 t.join();
	}

return 0;
}
























