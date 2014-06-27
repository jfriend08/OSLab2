/*
Usage: ./test [-v] -s<schedspec> inputfile randfile
--	it will generage the randNum based on CPUburst and IOburst
--	it will be able to detect -v or -s
--	do some test on switching states
--	priority queue for ready, run, and block states
--	create AllQ class, and many function in that class
--	now AllQ will dynamic to find the next event of task from every queue, and then switch it
--	cb vs remain when the task is almost done
--	doneP; modify CPU burst and IO burst; modify index/ofs for randFunction
--	now output is all the same as source except the last summary part

Todo:
--	define child classs, so it can run diff scheduler
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
#include <queue>

using namespace std;

vector<int> randvals;
int ofs=0, n1, n2, n3, n4;
int CPU_burst=10, IO_burst=10, CPUtime=0;
int c, vflag, sflag, opterr=0;
char *svalue=NULL;


////Function////
int myrandom(int burst, int &index) { 	
	if (index==randvals.size()){index=1;}
	index++;	
	// cout<<index<<":"<<randvals[index]<<endl;	
	return 1 + (randvals[index] % burst);	
}

string statecode(int i){
  static const char* statetr[] = {"RUNNG","BLOCK","READY", "NA"};
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
	int CPUB;
	int IOB;
	string curState;
	string nextState;	
	schedule(int ts,int pid,int tg, int re, int cpub, int iob,string cur,string next){
		// cout<<"constructor called"<<endl;
		Ts=ts; PID=pid; Tg=tg; remain=re; CPUB=cpub; IOB=iob; curState=cur; nextState=next;
	}
	~schedule(){};
	void Run2Block(int ts, int tg, int ib);
	void Block2Ready(int ts, int tg);
	void Ready2Run(int ts, int tg, int cb);
	void Run2Ready(int ts, int tg);
	void print(){cout<<"timestamp: "<<Ts<<" PID: "<<PID<<" Tg: "<<Tg<<" remain: "<<remain<<" curState: "<<curState<<" nextState: "<<nextState<<endl;}
	void enterReadyP(int &cputime){
		int dur=Ts-Tg;
		cout<<"==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
		cout<<"T("<<PID<<":"<<cputime<<"): "<<curState<<" -> "<<nextState<<endl<<endl;
	}
	void enterRunP(int &cputime){
		int dur=Ts-Tg; 
		cout<<"==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
		cout<<"T("<<PID<<":"<<cputime<<"): "<<curState<<" -> "<<nextState<<"  cb="<<Cb<<" rem="<<remain<<endl<<endl;
	}
	void enterBlockP(int &cputime){
		int dur=Ts-Tg; 
		cout<<"==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
		cout<<"T("<<PID<<":"<<cputime<<"): "<<curState<<" -> "<<nextState<<"  ib="<<Ib<<" rem="<<remain<<endl<<endl;
	}
	void doneP(int &cputime){
		int dur=Ts-Tg; 
		cout<<"==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
		cout<<"==> T("<<PID<<"): Done"<<endl<<endl;
		ofs=ofs-1;
	}
};


void schedule::Run2Block(int ts, int tg, int ib){
	Ts=ts; Tg=tg; Cb=-1000;Ib=ib;remain=remain-(Ts-Tg);curState=statecode(0); nextState=statecode(1);CPUtime=Ts;	
}
void schedule::Block2Ready(int ts, int tg){
	Ts=ts; Tg=tg; Cb=-1000;Ib=-1000;curState=statecode(1); nextState=statecode(2);CPUtime=Ts;
}
void schedule::Ready2Run(int ts, int tg, int cb){
	Ts=ts; Tg=tg; Cb=cb; Ib=-1000; remain=remain-(Ts-Tg) ;curState=statecode(2); nextState=statecode(0);CPUtime=Ts;
	if(remain<cb){Cb=remain;}
}
void schedule::Run2Ready(int ts, int tg){
	Ts=ts; Tg=tg; curState=statecode(0); nextState=statecode(2);CPUtime=Ts;		//in output5_R5_t the nextState is called PREEMPT
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


class Compare {
    public:
    bool operator()(schedule& t1, schedule& t2) // Returns true if t1 is earlier than t2
    { 
    	if (t1.PID > t2.PID) return true;
     	return false;
    }
};

class AllQ{
public:
	priority_queue<schedule, vector<schedule>, Compare> tasksQ;
	priority_queue<schedule, vector<schedule>, Compare> readyQ;
	priority_queue<schedule, vector<schedule>, Compare> runQ;
	priority_queue<schedule, vector<schedule>, Compare> blockQ;
	string smallest(){
		int s=10000; string Flag;
		if ((tasksQ.size()>0)&&(tasksQ.top().Ts<s)){
    		s=tasksQ.top().Ts;Flag="tasks"; 
    	}
		if ((readyQ.size()>0)&&(readyQ.top().Ts<s)){
    		s=readyQ.top().Ts;Flag="ready"; 
    	}
    	if((runQ.size()>0)&&(runQ.top().Ts<s)){
    		s=runQ.top().Ts;Flag="run"; 
    	}
    	if(((blockQ.size()>0))&&(blockQ.top().Ts<s)){
    		s=blockQ.top().Ts;Flag="block"; 
    	}
    	return Flag;
	}
	void change(){
		string tmpS=smallest();
		if (tmpS=="tasks"){
			schedule tmp=tasksQ.top();			
			if (tmp.remain>0){tmp.enterReadyP(tmp.Ts);readyQ.push(tmp);tasksQ.pop();} 
			else{tmp.doneP(CPUtime);tasksQ.pop();}			
		}
		if (tmpS=="ready"){
			schedule tmp=readyQ.top();
			tmp.Ready2Run(tmp.Ts, tmp.Ts, myrandom(tmp.CPUB,ofs));
			if (tmp.remain>0){tmp.enterRunP(CPUtime);runQ.push(tmp);readyQ.pop();}
			else{tmp.doneP(CPUtime);readyQ.pop();}			
		}
		if (tmpS=="run"){
			schedule tmp=runQ.top();
			tmp.Run2Block(tmp.Ts+tmp.Cb, tmp.Ts, myrandom(tmp.IOB,ofs));			
			if (tmp.remain>0){tmp.enterBlockP(CPUtime);blockQ.push(tmp);runQ.pop();;}
			else{tmp.doneP(CPUtime);runQ.pop();}
		}
		if (tmpS=="block"){
			schedule tmp=blockQ.top();
			tmp.Block2Ready(tmp.Ts+tmp.Ib, tmp.Ts);			
			if (tmp.remain>0){tmp.enterReadyP(CPUtime);readyQ.push(tmp);blockQ.pop();}
			else{tmp.doneP(CPUtime);blockQ.pop();}			
		}
	}
	bool stillRemain(){
		if((tasksQ.size()>0)|(readyQ.size()>0)|(runQ.size()>0)|(blockQ.size()>0)) return true;
		return false;
	}

};

// vector<schedule> tasksQ;
// vector<schedule> readyQ;



AllQ q;
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

	// pass input file
	ifstream fin0 ( argv[argc-2] );
	if (!fin0.is_open()){
		cout<<"Cannot open the file0"<<endl;}
	else{
		for (int i=0;!fin0.eof();i++){
			while(fin0>>n1>>n2 >>n3 >>n4){
				schedule t(n1,i,n1,n2,n3,n4,statecode(2),statecode(2));				
				(q.tasksQ).push(t);
				i++;
			}
		}
	}
	fin0.close();

	// pass the rand file
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

	// // test for the rand number generator
	// for (int i=0; i<randvals.size(); i++){		
	// 	cout<<myrandom(CPU_burst, ofs)<<endl;
	// }

	
	// schedule t1(0, 0, 0, 100, statecode(2), statecode(3));
	// t1.print();
	// t1.Ready2Run(t1.Ts, t1.Ts, myrandom(CPU_burst,ofs)); //(end, start, how long to run)
	// t1.print();
	// t1.Run2Block(t1.Ts+t1.Cb, t1.Ts, myrandom(IO_burst,ofs));
	// t1.print();
	// t1.Block2Ready(t1.Ts+t1.Ib, t1.Ts);
	// t1.print();
	
	// cout<<tasksQ.size()<<endl;
	// cout<<readyQ.size()<<endl;
	// cout<<runQ.size()<<endl;
	// cout<<blockQ.size()<<endl;


	// while(!q.tasksQ.empty()){
	// 	cout<<q.tasksQ.size()<<" "<<q.readyQ.size()<<endl;
	// 	schedule tmp=q.tasksQ.top();
	// 	// tmp.print();
	// 	// tmp.enterReadyP(CPUtime);
	// 	q.readyQ.push(tmp);
	// 	q.tasksQ.pop();
	// }

	while(q.stillRemain()){
		q.change();

	}
	cout<<q.tasksQ.size()<<" "<<q.readyQ.size()<<" "<<q.runQ.size()<<" "<<q.blockQ.size()<<endl;







	return 0;
}