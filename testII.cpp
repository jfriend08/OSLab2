/*
Usage: ./test [-v] -s<schedspec> inputfile randfile
--	it will generage the randNum based on CPUburst and IOburst
--	it will be able to detect -v or -s
--	do some test on switching states
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
#include <limits>

using namespace std;

vector<int> randvals;

////Function////
int myrandom(int burst, int &index) { 	
	if (index==randvals.size()){index=1;}
	index++;		
	return 1 + (randvals[index] % burst);	
}

string statecode(int i){
  static const char* statetr[] = {"RUNNG","BLOCKED","READY", "NA"};
  return statetr[i];
}


////Class////
class schedule{
public:
	int Ts; //time tp switch
	int PID; //id for this task
	int Tg; //time generate
	int remain; //remaining time
	int Cb;
	int Ib;
	string curState;
	string nextState;	
	schedule(int ts,int pid,int tg, int remain,string cur,string next);
	~schedule();
	void Run2Block(int ts, int tg, int ib);
	void Block2Ready(int ts, int tg);
	void Ready2Run(int ts, int tg, int cb);
	void Run2Ready(int ts, int tg);
	void print(){cout<<"timestamp: "<<Ts<<" PID: "<<PID<<" Tg: "<<Tg<<" remain: "<<remain<<" Cb: "<<Cb<<" Ib: "<<Ib<<" curState: "<<curState<<" nextState: "<<nextState<<endl;}
};
schedule::schedule(int ts, int pid, int tg, int re, string cur, string next){
	cout<<"constructor called"<<endl;
	Ts=ts; PID=pid; Tg=tg; remain=re; curState=cur; nextState=next;}

schedule::~schedule(){}

void schedule::Run2Block(int ts, int tg, int ib){
	Ts=ts; Tg=tg; Cb=-1000;Ib=ib;curState=statecode(0); nextState=statecode(1);	
}
void schedule::Block2Ready(int ts, int tg){
	Ts=ts; Tg=tg; Cb=-1000;Ib=-1000;curState=statecode(1); nextState=statecode(2);	
}
void schedule::Ready2Run(int ts, int tg, int cb){
	Ts=ts; Tg=tg; Cb=cb; Ib=-1000; curState=statecode(2); nextState=statecode(0);	
}
void schedule::Run2Ready(int ts, int tg){
	Ts=ts; Tg=tg; curState=statecode(0); nextState=statecode(2);		//in output5_R5_t the nextState is called PREEMPT
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
int CPU_burst=10, IO_burst=10;
int c, vflag, sflag, opterr=0;
char *svalue=NULL;

int main(int argc, char *argv[]){
	while((c=getopt(argc,argv,"vs:")) !=-1){
		cout<<c<<endl;
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
	// for (int i=0; i<10; i++){		
	// 	cout<<myrandom(CPU_burst, ofs)<<endl;
	// }

	cout<<"state: "<<statecode(0)<<" "<<statecode(1)<<" "<<statecode(2)<<endl;
	
	schedule t1(0, 0, 0, 100, statecode(2), statecode(3));
	t1.print();
	t1.Ready2Run(t1.Ts, t1.Ts, myrandom(CPU_burst,ofs)); //(end, start, how long to run)
	t1.print();
	t1.Run2Block(t1.Ts+t1.Cb, t1.Ts, myrandom(IO_burst,ofs));
	t1.print();
	t1.Block2Ready(t1.Ts+t1.Ib, t1.Ts);
	t1.print();






	return 0;
}