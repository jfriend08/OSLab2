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
--	able to print Per process information correctly; create Reporter map in report type to store those info
--	all sotring is still in schedule class, and all switching is still in AllQ class
--	able to print SUM. But aveIO might be wrong because you may have more than one tasks in IO?
--	can specific recheduler, and change the priority queue according. But havent test its accuracy
--	now can specify the -v flag. I am adding vflag value into those print function, and only print it if vflag==1
--	should probably calculat IOtime correct(?)

Todo:
--	have issue for input3. Which queue should really consider the insert index?
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
int CPU_burst=10, IO_burst=10, CPUtime=0, BlockEmptyTag=1;
double IOruntime=0,tmpIOruntime=0;
int c, vflag, sflag, opterr=0, RRval=-1;
string svalue,shedulFlag;



////Function////
int myrandom(int burst, int &index) { 	
	if (index==randvals.size()){index=1;}
	index++;	
	// cout<<index<<":"<<randvals[index]<<endl;	
	return 1 + (randvals[index] % burst);	
}
////Function////
string statecode(int i){
  static const char* statetr[] = {"RUNNG","BLOCK","READY", "NA"};
  return statetr[i];
}

////Class////
class report{
public:
	int At; //Arrive time
	int Tc; //Total CPU time
	int CPUB; //CPU burst
	int IOB; //IO burst
	int Ft; //Finising time
	int Tt; //Turnaround time (Ft - At)
	int It; //I/O time
	int Cw; //CPU waiting time (time in ready state)
	int cpuRun;
};


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
	string scheduler;
	int insertindex;
	int nextEventTime;
	map<int,report> Reporter;
	schedule();
	schedule(int ts,int pid,int tg, int re, int cpub, int iob,string cur,string next, string flag){
		Ts=ts; PID=pid; Tg=tg; remain=re; CPUB=cpub; IOB=iob; curState=cur; nextState=next; scheduler=flag;nextEventTime=Ts;
		Reporter[PID].At=Ts; Reporter[PID].Tc=remain; Reporter[PID].CPUB=CPUB; Reporter[PID].IOB=IOB;
		// cout<<PID<<" "<<Reporter[PID].At<<" "<<Reporter[PID].Tc<<" "<<Reporter[PID].CPUB<<" "<<Reporter[PID].IOB<<endl;
	}
	~schedule(){};
	void Run2Block(int ts, int tg, int ib);
	void Block2Ready(int ts, int tg, int num);
	void Ready2Run(int ts, int tg, int cb);
	void Run2Ready(int ts, int tg);
	void print(){cout<<"timestamp: "<<Ts<<" PID: "<<PID<<" Tg: "<<Tg<<" remain: "<<remain<<" curState: "<<curState<<" nextState: "<<nextState<<endl;}
	void enterReadyP(int &cputime, int vflag){
		int dur=Ts-Tg;
		if (vflag==1){
			cout<<"1==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
			cout<<"T("<<PID<<":"<<Ts<<"): "<<curState<<" -> "<<nextState<<endl<<endl;
			// cout<<"T("<<PID<<":"<<cputime<<"): "<<curState<<" -> "<<nextState<<endl<<endl;
		}
		
	}
	void enterRunP(int &cputime, int vflag){
		int dur=Ts-Tg; 
		if (vflag==1){
			cout<<"2==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
			cout<<"T("<<PID<<":"<<Ts<<"): "<<curState<<" -> "<<nextState<<"  cb="<<Cb<<" rem="<<remain<<endl<<endl;
			// cout<<"T("<<PID<<":"<<cputime<<"): "<<curState<<" -> "<<nextState<<"  cb="<<Cb<<" rem="<<remain<<endl<<endl;
		}		
	}
	void enterBlockP(int &cputime, int vflag){
		int dur=Ts-Tg; 
		if (vflag==1){
			cout<<"3==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
			cout<<"T("<<PID<<":"<<Ts<<"): "<<curState<<" -> "<<nextState<<"  ib="<<Ib<<" rem="<<remain<<endl<<endl;
			// cout<<"T("<<PID<<":"<<cputime<<"): "<<curState<<" -> "<<nextState<<"  ib="<<Ib<<" rem="<<remain<<endl<<endl;
		}		
	}
	void doneP(int &cputime, int vflag){
		int dur=Ts-Tg; 
		if (vflag==1){
			cout<<"==> "<<Ts<<" "<<PID<<" ts="<<Tg<<" "<< nextState<<"  "<<"dur="<<dur<<endl;
			cout<<"==> T("<<PID<<"): Done"<<endl<<endl;
			
		}
		ofs=ofs-1;
		Reporter[PID].Ft=Ts;
		Reporter[PID].Tt=Ts-Reporter[PID].At;
	}
	void currentP(){
		cout<<"Ts:"<<Ts<<" PID:"<<PID<<" Tg:"<<Tg<<" Remain:"<<remain<<" curState:"<<curState<<" nextState:"<<nextState<<" insertindex:"<<insertindex<<endl;
	}
};
void schedule::Run2Block(int ts, int tg, int ib){	
	remain=remain-(ts-tg);
	if(CPUtime<ts){CPUtime=ts;}else {ts=CPUtime; }
	Ts=ts; Tg=tg; Cb=-1000;Ib=ib;curState=statecode(0); nextState=statecode(1); nextEventTime=Ts+Ib;//CPUtime=Ts;		
	Reporter[PID].cpuRun=Reporter[PID].cpuRun+(Ts-tg);
}
void schedule::Block2Ready(int ts, int tg, int num){
	if(CPUtime<ts){CPUtime=ts;}else {ts=CPUtime; }
	Ts=ts; Tg=tg; Cb=-1000;Ib=-1000;curState=statecode(1); nextState=statecode(2); nextEventTime=Ts;//CPUtime=Ts;	
	Reporter[PID].It=Reporter[PID].It+(Ts-Tg);
}
void schedule::Ready2Run(int ts, int tg, int cb){
	remain=remain-(ts-tg);
	if(CPUtime<ts){CPUtime=ts;}else {ts=CPUtime; }
	Ts=ts; Tg=tg; Cb=cb; Ib=-1000;curState=statecode(2); nextState=statecode(0); nextEventTime=Ts+Cb;//CPUtime=Ts;
	if(remain<cb){Cb=remain;}
	Reporter[PID].Cw=Reporter[PID].Cw+(Ts-Tg);
}
void schedule::Run2Ready(int ts, int tg){
	if(CPUtime<ts){CPUtime=ts;}else {ts=CPUtime; }
	Ts=ts; Tg=tg; curState=statecode(0); nextState=statecode(2); //CPUtime=Ts;		//in output5_R5_t the nextState is called PREEMPT
}

////Class////
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

// ////Class////
// class Compare {
//     public:
//     virtual bool operator()(schedule& t1, schedule& t2) // Returns true if t1 is earlier than t2
//     { if (t1.PID > t2.PID) return true;return false;}        };
// class LCFSCompare {
//     public:
//     virtual bool operator()(schedule& t1, schedule& t2) // Returns true if t1 is earlier than t2
//     { if (t1.PID < t2.PID) return true;return false;}        };
// class SJFCompare {
//     public:
//     virtual bool operator()(schedule& t1, schedule& t2) // Returns true if t1 is earlier than t2
//     { if (t1.PID > t2.PID) return true;return false;}        };
class TaskCompare {
    public:
    virtual bool operator()(schedule& t1, schedule& t2) // Returns true if t1 is earlier than t2
    { if (t1.PID > t2.PID) return true;return false;}        };

////Class////
class ReportCompare {
    public:
    bool operator()(schedule& t1, schedule& t2) // Returns true if t1 is earlier than t2
    { if (t1.PID > t2.PID) return true;return false;}
};
class NexteventCompare {
    public:
    bool operator()(schedule& t1, schedule& t2) // Returns true if t1 is earlier than t2
    { if (t1.nextEventTime < t2.nextEventTime) return true;return false;}
};


////Class////
class AllQ{	
public:	
	class Compare {
    public:
    	bool operator()(schedule& t1, schedule& t2){
    		// if ((t1.scheduler=="F")&&(t1.PID > t2.PID)) return true;return false;
    		// if ((t1.scheduler=="L")&&(t1.PID < t2.PID)) return true;return false;
    		int flag=0;
    		if (t1.scheduler=="F"){
    			if (t1.insertindex > t2.insertindex) flag=1;    			    			
    		}
    		else if (t1.scheduler=="L"){
    			if (t1.insertindex < t2.insertindex) flag=1;
    		}
    		if (t1.scheduler=="R"){
    			if (t1.insertindex > t2.insertindex) flag=1;    			    			
    		}
    		if(flag==1) return true; return false;    		
		}            	
	};

	priority_queue<schedule, vector<schedule>, TaskCompare> tasksQ;
	priority_queue<schedule, vector<schedule>, Compare> readyQ;
	priority_queue<schedule, vector<schedule>, Compare> runQ;
	priority_queue<schedule, vector<schedule>, Compare> blockQ;
	
	priority_queue<schedule, vector<schedule>, ReportCompare> ReportQ;
	
	string smallest(string exclude){  //this is the function to find the queue who have the next cloest element		
		int s=10000;
		int tas=1 ,rea=1, run=1, blo=1;
		if (exclude=="tasks"){tas=0;}if (exclude=="ready"){rea=0;}if (exclude=="run"){run=0;}if (exclude=="block"){blo=0;}
		string Flag;
		if ((tas==1)&&(tasksQ.size()>0)&&(tasksQ.top().Ts<s)){s=tasksQ.top().Ts;Flag="tasks";   } // Flag2=Flag;Flag="tasks"; }
		if ((rea==1)&&(readyQ.size()>0)&&(readyQ.top().nextEventTime<s)){
		s=readyQ.top().nextEventTime; Flag="ready";		} // Flag2=Flag;Flag="ready"; }
    	if((run==1)&&(runQ.size()>0)&&(runQ.top().nextEventTime<s)){s=runQ.top().nextEventTime; Flag="run";} // Flag2=Flag;Flag="run"; }
    	if((blo==1)&&((blockQ.size()>0))&&(blockQ.top().nextEventTime<s)){s=blockQ.top().nextEventTime;Flag="block";} // Flag2=Flag;Flag="block"; }
    	return Flag;
	}
	int findLongEvent(priority_queue<schedule, vector<schedule>, Compare> q){
		priority_queue<schedule, vector<schedule>, Compare> qtmp;
		int longest=-1;
		qtmp=q;
		while (!qtmp.empty()){
			schedule t=qtmp.top();
			if (t.nextEventTime>longest){
				longest=t.nextEventTime;
			}
			qtmp.pop();
		}
		return longest;
	}
	int findLongEventIb(priority_queue<schedule, vector<schedule>, Compare> q){
		priority_queue<schedule, vector<schedule>, Compare> qtmp;
		int longest=-1;
		qtmp=q;
		while (!qtmp.empty()){
			schedule t=qtmp.top();
			if (t.nextEventTime>longest){
				longest=t.Ib;
			}
			qtmp.pop();
		}
		return longest;
	}
	int findLongIb(priority_queue<schedule, vector<schedule>, Compare> q){
		priority_queue<schedule, vector<schedule>, Compare> qtmp;
		int longest=-1;
		qtmp=q;
		while (!qtmp.empty()){
			schedule t=qtmp.top();
			if (t.Ib>longest){
				longest=t.Ib;
			}
			qtmp.pop();
		}
		return longest;
	}
	int findLongTs(priority_queue<schedule, vector<schedule>, Compare> q){
		priority_queue<schedule, vector<schedule>, Compare> qtmp;
		int longest=-1;
		qtmp=q;
		while (!qtmp.empty()){
			schedule t=qtmp.top();
			if (t.Ib>longest){
				longest=t.Ts;
			}
			qtmp.pop();
		}
		return longest;
	}

	int Qlastindex(priority_queue<schedule, vector<schedule>, Compare> q){
		priority_queue<schedule, vector<schedule>, Compare> qtmp;
		int index=-1;
		qtmp=q;
		while (!qtmp.empty()){
			schedule t=qtmp.top();
			index=t.insertindex;
			qtmp.pop();
		}
		return index+1;
	}
	
	void IOtimeCalculation(){
		if ((tmpIOruntime==0)&&(blockQ.size()>0)){
			IOruntime=findLongIb(blockQ);			
			tmpIOruntime=findLongEvent(blockQ);			
		}
		else if ((tmpIOruntime<CPUtime)&&(blockQ.size()>0)){						
			if(BlockEmptyTag==1){
				double longest=findLongEvent(blockQ);
				double longest_Ib=findLongEventIb(blockQ);				
				// cout<<">>>>>>>>1 longest:"<<longest<<" longest_Ib:"<<longest_Ib<<endl;
				IOruntime=IOruntime+longest_Ib;
				tmpIOruntime=longest;
				// cout<<">>>>>>>>1 IOruntime:"<<IOruntime<<" tmpIOruntime:"<<tmpIOruntime<<endl;
				BlockEmptyTag=0;
			}
			else if(BlockEmptyTag==0){
				double longest=findLongEvent(blockQ);
				// cout<<">>>>>>>>2 longest:"<<longest<<" tmpIOruntime:"<<tmpIOruntime<<endl;
				IOruntime=IOruntime+(longest-tmpIOruntime);
				tmpIOruntime=longest;
				// cout<<">>>>>>>>2 IOruntime:"<<IOruntime<<" tmpIOruntime:"<<tmpIOruntime<<endl;
				BlockEmptyTag=0;
			}
		}		
		else if(blockQ.size()==0) {BlockEmptyTag=1;}
	}

	void change(){  // this is the function for state switching which remain>0, otherwise it will print done.
		string tmpS=smallest("NA");	
		if ((runQ.size()>0)&&(tmpS=="ready")){
			tmpS=smallest(tmpS);
		}			

		if (tmpS=="tasks"){
			schedule tmp=tasksQ.top();			
			if (tmp.remain>0){tmp.enterReadyP(tmp.Ts, vflag);tmp.insertindex=Qlastindex(readyQ);readyQ.push(tmp);tasksQ.pop();} 
			else{tmp.doneP(CPUtime, vflag);ReportQ.push(tmp);tasksQ.pop();}			}
		if (tmpS=="ready"){
			schedule tmp=readyQ.top();
			tmp.Ready2Run(tmp.Ts, tmp.Ts, myrandom(tmp.CPUB,ofs));
			if (tmp.remain>0){tmp.enterRunP(CPUtime, vflag);tmp.insertindex=Qlastindex(runQ);runQ.push(tmp);readyQ.pop();}
			else{
				tmp.doneP(CPUtime, vflag);ReportQ.push(tmp);readyQ.pop();}			}
		if (tmpS=="run"){
			schedule tmp=runQ.top();
			tmp.Run2Block(tmp.Ts+tmp.Cb, tmp.Ts, myrandom(tmp.IOB,ofs));			
			if (tmp.remain>0){tmp.enterBlockP(CPUtime, vflag);tmp.insertindex=Qlastindex(blockQ);blockQ.push(tmp);runQ.pop();;}
			else{
				tmp.doneP(CPUtime, vflag);ReportQ.push(tmp);runQ.pop();}}
		if (tmpS=="block"){
			schedule tmp=blockQ.top();
			tmp.Block2Ready(tmp.Ts+tmp.Ib, tmp.Ts, blockQ.size());			
			if (tmp.remain>0){tmp.enterReadyP(CPUtime, vflag);tmp.insertindex=Qlastindex(readyQ);readyQ.push(tmp);blockQ.pop();}
			else{
				tmp.doneP(CPUtime, vflag);ReportQ.push(tmp);blockQ.pop();}			}				
	}

	bool stillRemain(){  // this is the function testing there are still element in all the queue, otherwise return false
		if((tasksQ.size()>0)|(readyQ.size()>0)|(runQ.size()>0)|(blockQ.size()>0)) return true;
		return false;
	}

	void printReport(){
		int FinishTime=0;
		double aveCPU=0,aveIO=0,aveTt=0,aveCw=0,aveThrouput=0,numTask=ReportQ.size();		
		while(!ReportQ.empty()){
			schedule tmp=ReportQ.top();
			printf("%04d: %4d %4d %4d %4d | %4d %4d %4d %4d\n", tmp.PID,tmp.Reporter[tmp.PID].At, tmp.Reporter[tmp.PID].Tc
				, tmp.Reporter[tmp.PID].CPUB, tmp.Reporter[tmp.PID].IOB, tmp.Reporter[tmp.PID].Ft, tmp.Reporter[tmp.PID].Tt, tmp.Reporter[tmp.PID].It, tmp.Reporter[tmp.PID].Cw);			
			if (tmp.Reporter[tmp.PID].Ft>FinishTime){FinishTime=tmp.Reporter[tmp.PID].Ft;}
			aveCPU=aveCPU+tmp.Reporter[tmp.PID].cpuRun;
			aveIO=aveIO+tmp.Reporter[tmp.PID].It;
			aveTt=aveTt+tmp.Reporter[tmp.PID].Tt;
			aveCw=aveCw+tmp.Reporter[tmp.PID].Cw;
			ReportQ.pop();
		}
		aveCPU=aveCPU/FinishTime*100;
		// aveIO=aveIO/FinishTime*100;		
		aveIO=(IOruntime/FinishTime)*100;		
		aveTt=aveTt/numTask;
		aveCw=aveCw/numTask;
		aveThrouput=numTask/FinishTime*100;
		printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", FinishTime ,aveCPU, aveIO, aveTt, aveCw, aveThrouput);
	}
	void qReport(){
		deque<int> q;		
		if(tasksQ.size()>0){
			schedule task=tasksQ.top();
			cout<<"Task: topPID:"<<task.PID<<" task.Ts:"<<task.Ts<<" task.nextEventTime:"<<task.nextEventTime<<" tasksize:"<<tasksQ.size()<<endl;
		}			
		if(readyQ.size()>0){
			schedule ready=readyQ.top();
			cout<<"Ready: topPID:"<<ready.PID<<" ready.Ts:"<<ready.Ts<<" ready.nextEventTime:"<<ready.nextEventTime<<" readysize:"<<readyQ.size()<<endl;
		}
		if(runQ.size()>0){
			schedule run=runQ.top();
			cout<<"Run: topPID:"<<run.PID<<" run.Ts:"<<run.Ts<<" run.nextEventTime:"<<run.nextEventTime<<" runsize:"<<runQ.size()<<endl;
		}
		if(blockQ.size()>0){
			schedule block=blockQ.top();
			cout<<"Block: PID:"<<block.PID<<" block.Ts:"<<block.Ts<<" block.nextEventTime:"<<block.nextEventTime<<" blocksize:"<<blockQ.size()<<endl;
		}				
		cout<<endl;
	}
};


AllQ q;

int main(int argc, char *argv[]){
	while((c=getopt(argc,argv,"vs:")) !=-1){
		switch (c){
			case 'v':
				vflag=1;
				break;
			case 's':
				sflag=1;
				svalue.assign(optarg);
				break;
			default:
				abort();
		}
	}
	// cout<<"vflag="<<vflag<<" sflag="<<sflag<<" svalue="<<svalue<<endl;
	
	if (svalue[0]=='R'){
		string tmp=svalue.substr(1,svalue.size());
		istringstream buffer(tmp);
		buffer >> RRval;		
		svalue=svalue[0];
	}
	
	// pass input file
	ifstream fin0 ( argv[argc-2] );
	if (!fin0.is_open()){
		cout<<"Cannot open the file0"<<endl;}
	else{
		for (int i=0;!fin0.eof();i++){			
			while(fin0>>n1>>n2 >>n3 >>n4){
				schedule t(n1,i,n1,n2,n3,n4,statecode(2),statecode(2),svalue);								
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
	// int test=0;
	// while(test<10){
	while(q.stillRemain()){
		q.qReport();
		q.change();
		q.IOtimeCalculation();


	}

	// cout<<q.tasksQ.size()<<" "<<q.readyQ.size()<<" "<<q.runQ.size()<<" "<<q.blockQ.size()<<" "<<q.ReportQ.size()<<endl;

	// for (int i=0;i<q.ReportQ.size();i++){
	// 	schedule tmp=q.ReportQ.top();
	// 	// cout<<tmp.Reporter[i].At<<endl;
	// 	tmp.ReportQ.pop();
	// }

	if (svalue=="F"){cout<<"FCFS"<<endl;}
	else if(svalue=="F"){cout<<"LCFS"<<endl;}
	else if (svalue=="S"){cout<<"SJF"<<endl;}
	else if (svalue=="R"){cout<<"RR "<<RRval<<endl;}
	q.printReport();







	return 0;
}