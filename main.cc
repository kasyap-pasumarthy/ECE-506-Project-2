/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>

#include <assert.h>

#include <fstream>

#include <string>

#include <iostream>

#include <math.h>

#include <sstream>

#include <bitset>

#include <math.h>

#include <cstring>

#include <sstream>

#include <stdio.h>

#include <iomanip>


using namespace std;

#include "cache.h"

int main(int argc, char *argv[])
{
	
	ifstream fin;
	//FILE * pFile;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
	string fname = argv[6];

	cout << "===== 506 Personal information =====" << endl;
	cout << "Viswanatha Kasyap Pasumarthy" << endl;
	cout << "UnityID - 200310870" << endl;
	cout << "ECE 492 Students? NO" << endl;
	
	//****************************************************//
	//**printf("===== Simulator configuration =====\n");**//
	//*******print out simulator configuration here*******//
	//****************************************************//
	cout << "===== 506 SMP Simulator configuration =====" << endl;
	cout << "L1_SIZE: " <<  cache_size << endl;
	cout << "L1_ASSOC: "<< cache_assoc << endl;
	cout << "L1_BLOCKSIZE: " << blk_size << endl;
	cout << "NUMBER OF PROCESSORS: " << num_processors << endl;
	if (protocol == 0 )
	cout << "COHERENCE PROTOCOL: MSI" << endl;
	else if (protocol == 1)
	cout << "COHERENCE PROTOCOL: MESI" << endl;
	else if (protocol == 2)
	cout << "COHERENCE PROTOCOL: Dragon" << endl; 
	cout << "TRACE FILE: " << fname << endl;
 
	//*********************************************//
        //*****create an array of caches here**********//
	//*********************************************//	

	Cache **arrayOfCaches = new Cache *[num_processors];

	for(int i=0; i<num_processors; i++){
		arrayOfCaches[i] = new Cache(cache_size, cache_assoc, blk_size, num_processors, arrayOfCaches);
	}



	//pFile = fopen (fname,"r");
	//if(pFile == 0)
	//{   
	//	printf("Trace file problem\n");
	//	exit(0);
	//}
	string sub1, sub2, sub3;
	ulong address;
	int processor =0;
	string line;
	ifstream infile;
	infile.open(fname.c_str());
	
	
	while (getline(infile, line)){
	istringstream s(line);
 	s >> sub1; // processor number
	s >> sub2; // op type
	s >> sub3; // address
	istringstream buffer(sub3);
	istringstream buffer1(sub1);


	buffer >> hex >> address;
	buffer1 >> dec >> processor;

	//cout<<"proc " << processor << " op " << sub2.c_str() << " ADDress "<<address<<endl;
	if(protocol == 0){
	arrayOfCaches[processor] -> MSI(processor, address, sub2.c_str());
	}
	else if(protocol == 1){
	arrayOfCaches[processor] -> MESI(processor, address, sub2.c_str());
	}
	else if(protocol == 2){
	arrayOfCaches[processor] -> DRAGON(processor, address, sub2.c_str());
	}

	}
	infile.close();
	
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cachesArray[processor#]->Access(...)***************//
	///******************************************************************//
	//fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//
	
for(int i =0; i< num_processors; i++){


cout<<"============ Simulation results (Cache "<<i<<") ============"<<endl;

ulong memRead;

if(protocol == 0){
memRead = arrayOfCaches[i]->memReads +arrayOfCaches[i]->readMisses + arrayOfCaches[i]->writeMisses + arrayOfCaches[i]->writeBacks;
}
else if(protocol == 1){
memRead =arrayOfCaches[i]->readMisses+arrayOfCaches[i]->writeMisses+ arrayOfCaches[i]->writeBacks  - arrayOfCaches[i]->cachetocache;
}

else if(protocol == 2){
memRead =arrayOfCaches[i]->readMisses + arrayOfCaches[i]->writeMisses + arrayOfCaches[i]->writeBacks;
}

arrayOfCaches[i]->printStats(memRead);
}


}
