#include <iostream>
#include <thread>
#include <unistd.h>
#include <fstream>
#include <random>
#include <vector>
#include <iomanip>
#include "semaphore.h"
#include<sstream>

using namespace std;

default_random_engine Generator(time(NULL));        //generates random number
exponential_distribution<double> *distribution1,*distribution2;     //exponential distributuons
time_t t;   
vector<string> Time;    //globally declared buffer to store all times at which request, given access, left Time called.
vector<double> waitingTime;


int eating = 0, waiting = 0; //eating and waiting keep track of the number of threads sitting at the table and waiting respectively.
sem_t mutex, block; //incoming customers wait on block
bool  must_wait = false; //must_wait indicates that the table is full
ofstream Log("Log-CS19BTECH11026.txt");


class Info {
    public:
        int n;  //number of people
        int x;     //number of seats
        double lambda;
        double r;
        double tau;

        void getParameter();
} information;

void Info::getParameter() { //takes input from the input.txt file
    ifstream input;
    input.open("input.txt");
    input>> n>> x>> lambda>> r>> tau;
    input.close();
}

string getSysTime() {   //used to get system time
	time(&t);
	struct tm * Time;
	Time = localtime (&t);
	char String[9];
	sprintf(String, "%.2d:%.2d:%.2d", Time->tm_hour, Time->tm_min, Time->tm_sec);
	string str(String);
	return str;
}

void Korean(int id) {
    chrono::time_point<chrono::system_clock> startTime,endTime;
    stringstream ss;    //used to convert int (id+1) into string 
    ss<< id+1;
    string s;
    ss>> s;

    startTime = chrono::system_clock::now();
    Time.push_back(s+"th customer access request at time: "+getSysTime());  //here customer request so pushing here's time into vector Time
    sem_wait(&mutex);
    
    if(must_wait==true || eating+1 > information.x ) {
        waiting++;
        must_wait = true;
        sem_post(&mutex);
        sem_post(&block);
    }
        
    else {
        eating++;
        must_wait = (waiting>0 && eating == information.x);
        sem_post(&mutex);
    }
    Time.push_back(s+"th given access at time: "+getSysTime()); //here customer given access so pushing into vector Time
    endTime = chrono::system_clock::now();
    
    waitingTime.push_back(chrono::duration_cast <chrono::microseconds> (endTime-startTime).count());
    distribution2 = new exponential_distribution<double>(information.tau);
    usleep((*distribution2)(Generator)*1000000);     
    
    sem_wait(&mutex);
    Time.push_back(s+"th left at time: "+getSysTime()); //here customer left the table
        
    eating--;
    if(eating == 0) {
        int k = min(information.x , waiting);
        waiting -= k;
        eating += k;
        must_wait = ( waiting>0 && eating == information.x);
        
        for(int i=0; i<k; i++)
            sem_post(&block);
    }
    sem_post(&mutex); 
}


int main(void) {
    information.getParameter();     // getting input
    vector<thread> Threads;     // number of people

    sem_init(&mutex, 0, 1);
    sem_init(&block, 0, 0);
    double s = (double)1 + (double)(rand()) / ((double)(RAND_MAX/((double)information.r*(double)information.x - (double)1)));
    int track = 0;

    while(track < information.n) {  //until track less than total number of people
        distribution1 = new exponential_distribution<double>(information.lambda);
        usleep((*distribution1)(Generator)*1000000);    

        for(int i=track; i<(int)(s); i++) {
            Threads.push_back(thread(Korean, i));       //creating thread
        } 
        
        for(int i=track; i<(int)(s); i++) {
            Threads[i].join();  //joining threads
        } 
        track = (int)s;

        while(track+int(s)>information.n && track<information.n) {
            s = (double)1 + (double)(rand()) / ((double)(RAND_MAX/((double)information.r*(double)information.x - (double)1)));
        }
        s = (double)track + (double)1 + (double)(rand()) / ((double)(RAND_MAX/((double)information.r*(double)information.x - (double)1)));
    }

    for(int i=0; i<Time.size(); i++) {
        Log<< Time[i]<< endl;
    }

    double totalWaitingTime = 0;

    for(int i=0; i<waitingTime.size(); i++) {
        totalWaitingTime += waitingTime[i];
    }
    cout<< "Average Waiting Time = "<< totalWaitingTime/(double)information.n << "us"<<endl;
    cout<<"For Log file open Log-CS19BTECH11026.txt\n";
}