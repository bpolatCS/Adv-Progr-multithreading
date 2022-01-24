#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>
#include <chrono>
#include "bpolat_sarpbora_polat_hw7_DynIntQueueClass.h"
#include <iomanip>
#include <random>
#include <time.h>
using namespace std;

int total_box_count, min_production, max_production; //total box count and producer's working range
int min_filling_time, max_filling_time, min_packaging_time, max_packaging_time; //for the random time variable each filler and packager
int packagerCounter = 0; //for while loop counter of packager function
int fillerCounter = 0; //for while loop
mutex fillMutex, pacMutex, prodMutex, coutMutex; //mutexes for each functions and for one cout
HW7DynIntQueue fillQue, pacQue;


//Taken by homework 7 document
int random_range(const int & min, const int & max) 
{ 
	static mt19937 generator(time(0)); 
	uniform_int_distribution<int> distribution(min, max); 
	return distribution(generator); 
}
//Taken by homework 7 document

void producer()
{	
	for(int i = 1; i <= total_box_count; i++) //for each box there will be a loop that enques the box to the fillQue and print corresponding statement. 
											
	{
		time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
		struct tm *ptm = new struct tm; 

		this_thread::sleep_for(chrono::seconds(random_range(min_production,max_production))); //working time

		prodMutex.lock();	
		fillQue.enqueue(i); //each deque and enque process mutex should be open and close
		prodMutex.unlock();

		coutMutex.lock();
		tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 		
		localtime_s(ptm, &tt);
		cout <<"Producer has enqueued a new box "<< i << " to filling queue (filling queue size is " << fillQue.getCurrentSize() << " ): "<< put_time(ptm,"%X") <<endl;	

		coutMutex.unlock();	

	}

}

void filler(int consId)
{
	int item;

	while(fillerCounter < total_box_count) //global filler counter for looping each box. 
	{
		fillMutex.lock();
		while (fillQue.isEmpty())
		{
			fillMutex.unlock();
			fillMutex.lock();   //here this while loop is the critical part. In order to wait producer's enqueue box we have a switch key. Something like waiting room. When fillQUe is empty, thread should wait.

			if (fillerCounter == total_box_count) //this is the guard for the last box. When all boxes are dequeued from fillQue, there will be empty. But still a thread have to wait to become not empty. So it will enter infinite loop.
			{
				fillMutex.unlock();
				return;
			}						
		}

		fillQue.dequeue(item); // filler dequeued box 
		fillerCounter++;
		fillMutex.unlock();			

		coutMutex.lock();
		time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 
		struct tm *ptm = new struct tm; 
		localtime_s(ptm, &tt);
		cout<<"Filler "<<consId<<" started filling the box "<< item <<" to filling queue (filling queue size is " << fillQue.getCurrentSize() << " ): "<< put_time(ptm,"%X") <<endl;	
		coutMutex.unlock();

		this_thread::sleep_for(chrono::seconds(random_range(min_filling_time,max_filling_time)));

		coutMutex.lock();
		tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 		
		localtime_s(ptm, &tt);
		cout<<"Filler "<<consId<<" finished filling the box "<< item <<": "<< put_time(ptm,"%X")<<endl;
		coutMutex.unlock();

		this_thread::sleep_for(chrono::seconds(random_range(min_filling_time,max_filling_time)));

		pacMutex.lock();
		pacQue.enqueue(item); // here dequeued box will enqueue to pacQue
		pacMutex.unlock();

		coutMutex.lock();
		tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 		
		localtime_s(ptm, &tt);
		cout<<"Filler "<<consId<<" put box "<< item << " packaging queue (packaging queue size is "<< pacQue.getCurrentSize() << " ): "<< put_time(ptm,"%X")<<endl;
		
		coutMutex.unlock();


		
				
	}
	
}

void packager(int consId)
{
	int item;
	int counter = 0;

	while(packagerCounter < total_box_count)
	{
		pacMutex.lock();

		while (pacQue.isEmpty()) // here this while loop is the critical part. In order to wait filler's enqueue box to packQue we have a switch key. 
								// Something like waiting room. When packQUe is empty, thread should wait.
		{
			pacMutex.unlock();
			pacMutex.lock(); 

			if (packagerCounter == total_box_count) // this is the guard for the last box. When all boxes are dequeued from fillQue, there will be empty. 
													// But still a thread have to wait to become not empty. So it will enter infinite loop.
			{
				pacMutex.unlock();
				return;
			}
		}


	
		pacQue.dequeue(item); //packager finished its job and de
		packagerCounter++;
		pacMutex.unlock();
		coutMutex.lock();

		time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 
		struct tm *ptm = new struct tm; 
		localtime_s(ptm, &tt);
		cout<<"Packager "<<consId<<" started packaging the box: "<< item <<" (packaging queue size is " << pacQue.getCurrentSize() << " ): "<< put_time(ptm,"%X") <<endl;	
		coutMutex.unlock();

		this_thread::sleep_for(chrono::seconds(random_range(min_packaging_time,max_packaging_time)));

		coutMutex.lock();
		tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 
		localtime_s(ptm, &tt);
		cout<<"Packager "<<consId<<" finished packaging the box: "<< item <<": "<< put_time(ptm,"%X")<<endl;
		coutMutex.unlock();		
		

	}
}


int main()
{
	cout<<"Please enter the total number of items: ";
	cin>>total_box_count;

	cout<<"Please enter the min-max waiting time range of producer: "<<endl<<"Min: ";	
	cin>>min_production;
	cout<<"Max: ";
	cin>>max_production;

	cout<<"Please enter the min-max waiting time range of filler workers: "<<endl<<"Min: ";	
	cin>>min_filling_time;
	cout<<"Max: ";
	cin>>max_filling_time;

	cout<<"Please enter the min-max waiting time range of packager workers: "<<endl<<"Min: ";	
	cin>>min_packaging_time;
	cout<<"Max: ";
	cin>>max_packaging_time;

	//taken from lecture materials
	time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 
	struct tm *ptm = new struct tm; 
	localtime_s(ptm, &tt);
	cout<<"Simulation starts "<< put_time(ptm,"%X")<<endl;
	//taken from lecture materials

	thread producer(&producer);
	thread filler1(&filler, 1); //creation process
	thread filler2(&filler, 2);
	thread packager1(&packager, 1);
	thread packager2(&packager, 2);

	producer.join();
	filler1.join();
	filler2.join();  //at the and join all threads
	packager1.join();
	packager2.join();
	
	tt = chrono::system_clock::to_time_t(chrono::system_clock::now()); 
	localtime_s(ptm, &tt);
	cout<<"End of the simulation ends: "<< put_time(ptm,"%X")<<endl;

	delete ptm;
	return 0;
}
