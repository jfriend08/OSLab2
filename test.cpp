/*
Usage: ./test rfile
It will generage the randNum based on CPUburst and IOburst
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
#include <stdlib.h>

using namespace std;

vector<int> randvals;
int ofs=0;
int CPU_burst=10, IO_burst;

int myrandom(int burst, int &index) { 	
	if (index==randvals.size()){index=1;}
	index++;		
	return 1 + (randvals[index] % burst);	
}

int main(int argc, char *argv[]){
	ifstream fin ( argv[1] ); // processing the rand file
	if (!fin.is_open()){
		cout<<"Cannot open the file"<<endl;}
	else{
		for (int i=0; !fin.eof();i++){
			int temp;
			fin>>temp;
			randvals.push_back(temp);
		}
	}

	// test for the rand number generator
	for (int i=0; i<10; i++){		
		cout<<myrandom(CPU_burst, ofs)<<endl;
	}
}