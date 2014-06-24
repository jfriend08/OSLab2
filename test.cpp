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

////Class////
class schedule{
public:
	int timestamp; //start time
	int PID; //id for this task
	int Ts; //generated time
	int Dur;
	string curState;
	string nextState;	
	schedule(int t,int pid,int ts,int dur,string cur,string next);
	~schedule();
};
schedule::schedule(int t,int pid,int ts,int dur,string cur,string next){
	cout<<"constructor called"<<endl;
	timestamp=t; PID=pid; Ts=ts; Dur=dur; curState=cur; nextState=next;}
schedule::~schedule(){}

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
	








	return 0;
}