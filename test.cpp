/*
Usage: ./test [-v] [-s<schedspec>] inputfile randfile
--	it will generage the randNum based on CPUburst and IOburst
--	it will be able to detect -v or -s
*/

#include <fstream>
#include <iostream>
#include <string>
#include <vector> 
#include <list>
#include <sstream>
#include <iterator>
#include <map> 
#include <iomanip>
#include <utility> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

using namespace std;

vector<int> randvals;

////Function////
int myrandom(int burst, int &index) { 	
	if (index==randvals.size()){index=1;}
	index++;		
	return 1 + (randvals[index] % burst);	
}

string statecode(int i){
  static const char* statetr[] = {"RUNNG","BLOCKED","READY"};
  return statetr[i];
}

////Class////
class schedule{
public:
	int timestamp; //start time
	int PID; //id for this task
	int Ts; //generated time
	int remain; //remaining time
	int Cb;
	int Ib;
	string curState;
	string nextState;	
	schedule(int t,int pid,int ts, int remain,string cur,string next);
	~schedule();
	void Run2Block(int t, int ts);
	void Block2Ready(int t, int ts);
	void Ready2Run(int t, int ts, int cb);
	void Run2Ready(int t, int ts);
};
schedule::schedule(int t, int pid, int ts, int re, string cur, string next){
	cout<<"constructor called"<<endl;
	timestamp=t; PID=pid; Ts=ts; remain=re; curState=cur; nextState=next;}

schedule::~schedule(){}

void schedule::Run2Block(int t, int ts){
	timestamp=t; Ts=ts; curState=statecode(0); nextState=statecode(1);	
}
void schedule::Block2Ready(int t, int ts){
	timestamp=t; Ts=ts; curState=statecode(1); nextState=statecode(2);	
}
void schedule::Ready2Run(int t, int ts, int cb){
	timestamp=t; Ts=ts; Cb=cb; curState=statecode(2); nextState=statecode(0);	
}
void schedule::Run2Ready(int t, int ts){
	timestamp=t; Ts=ts; curState=statecode(0); nextState=statecode(2);		//in output5_R5_t the nextState is called PREEMPT
}

class inputTask{
public:
	int AT; //arrivetime
	int TC; //total cpu time
	int CB; //cpu burst
	int IO; //io burst
	inputTask(int at, int tc, int cb, int io);
	~inputTask();
};
inputTask::inputTask(int at, int tc, int cb, int io){AT=at;TC=tc;CB=cb;IO=io;}
inputTask::~inputTask(){}

vector<inputTask>tasks_v;
int ofs=0, n1, n2, n3, n4;
int CPU_burst=10, IO_burst;
int c, vflag, sflag;
char *svalue=NULL;

int main(int argc, char *argv[]){
	while((c=getopt(argc,argv,"vs:")) !=-1){
		switch (c){
			case 'v':
				vflag=1;
				break;
			case 's':
				sflag=1;
				svalue=optarg;
				break;
			default:
				abort();
		}
	}
	cout<<"vflag="<<vflag<<" sflag="<<sflag<<" svalue="<<svalue<<endl;
	
	// processing input file
	ifstream fin0 ( argv[argc-2] );
	if (!fin0.is_open()){
		cout<<"Cannot open the file0"<<endl;}
	else{
		for (int i=0;!fin0.eof();i++){
			while(fin0>>n1>>n2 >>n3 >>n4){
				inputTask elem(n1, n2, n3, n4);
				tasks_v.push_back(elem);
			}
		}
	}
	fin0.close();

	// test task input
	for (int i=0;i<tasks_v.size();i++){
		cout<<tasks_v[i].AT<<" "<<tasks_v[i].TC<<" "<<tasks_v[i].CB<<" "<<tasks_v[i].IO<<" "<<endl;
	}

	// processing the rand file
	ifstream fin ( argv[argc-1] ); 
	if (!fin.is_open()){
		cout<<"Cannot open the file1"<<endl;}
	else{
		for (int i=0; !fin.eof();i++){
			int temp;
			fin>>temp;
			randvals.push_back(temp);
		}
	}
	fin.close();

	// test for the rand number generator
	for (int i=0; i<10; i++){		
		cout<<myrandom(CPU_burst, ofs)<<endl;
	}

	cout<<"state: "<<statecode(0)<<" "<<statecode(1)<<" "<<statecode(2)<<endl;
	








	return 0;
}