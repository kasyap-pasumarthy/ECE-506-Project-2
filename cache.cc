/*******************************************************
                          cache.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b, int processors, Cache **arrayofcaches )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
	memReads = 0;
  	procs = processors;
	
   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  arrayOfCaches = arrayofcaches;
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }      
   
}





/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/


void Cache::MSI(int current_processor,ulong address, const char* op){ // start MSI

//cout <<"MSI CALLED"<<endl;

current_proc = current_processor;
currentCycle++;

cacheLine * currentLine = findLine(address);

if(*op == 'r'){
reads++;
//cout <<"READ CALLED"<<endl;

//read miss
if(currentLine == NULL){
//cout <<"line 74"<<endl;
readMisses++;

cacheLine * nextLine = fillLine(address);
nextLine -> setFlags(SHARED);

//issue bus read

for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "b";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->MSI(i, address, op1.c_str());


}// end bus read

//cout <<"line 96"<<endl;

}// end read miss

else {//read hit

updateLRU(currentLine);

if(currentLine -> getFlags() == MODIFIED){

currentLine->setFlags(MODIFIED);

}
if(currentLine -> getFlags() == SHARED){
currentLine->setFlags(SHARED);
}
}//end read hit


}//end read

if(*op == 'w'){
//cout <<"Write CALLED"<<endl;
writes++;

//write miss
if(currentLine == NULL){

writeMisses++;

cacheLine * nextLine = fillLine(address);
nextLine -> setFlags(MODIFIED);

//issue bus read X
BusRdX++;
for(int i=0; i<procs; i++){

if(i == current_proc){
continue;
}

string op1 = "x";

arrayOfCaches[i]->MSI(i, address, op1.c_str());


}// end bus read X



}// end write miss

else {//write hit

updateLRU(currentLine);

if(currentLine -> getFlags() == MODIFIED){

currentLine->setFlags(MODIFIED);

}
if(currentLine -> getFlags() == SHARED){
currentLine->setFlags(MODIFIED);

BusRdX++;
memReads++;
for(int i=0; i<procs; i++){

if(i == current_proc){
continue;
}

string op1 = "x";

arrayOfCaches[i]->MSI(i, address, op1.c_str());


}// end bus read X
}


}//end write hit



}//endwrite

if(*op == 'b'){
if(currentLine == NULL){
}
else
{
//cout <<"bus read line 184 CALLED"<<endl;
if(currentLine -> getFlags() == MODIFIED){
//cout <<"line 186 CALLED"<<endl;
currentLine -> setFlags(SHARED);
interventions++;

writeBack(address);
flushes++;

}

if(currentLine -> getFlags() == SHARED){
//cout <<"line 196 CALLED"<<endl;

currentLine -> setFlags(SHARED);

}
}

}//end BusRd

if(*op == 'x'){

if(currentLine == NULL){

}

else{
if(currentLine -> getFlags() == MODIFIED){

currentLine -> setFlags(INVALID);
invalidations++;

writeBack(address);
flushes++;

}

if(currentLine -> getFlags() == SHARED){


currentLine -> setFlags(INVALID);
invalidations++;

}
}
}//end BusRdX
}// end MSI

void Cache::MESI(int current_processor,ulong address, const char* op){ // start MESI

//cout <<"MESI CALLED"<<endl;

current_proc = current_processor;
currentCycle++;

cacheLine * currentLine = findLine(address);

int single;

for(int i =0; i< procs; i++){

if(i == current_proc)
continue;

single = arrayOfCaches[i]->searchCache(address);
if(single == 1)
break;


}


if(*op == 'r'){
reads++;
if(currentLine == NULL){//read miss
readMisses++;
cacheLine * nextLine = fillLine(address);

if(single ==0){

nextLine ->setFlags(EXCLUSIVE);

}

else if (single == 1){

nextLine->setFlags(SHARED);
cachetocache++;

}

for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "b";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->MESI(i, address, op1.c_str());


}// end bus read

}//read miss


else {//read hit

updateLRU(currentLine);

if(currentLine->getFlags() == MODIFIED){
currentLine -> setFlags(MODIFIED);
}


else if(currentLine->getFlags() == SHARED){

currentLine -> setFlags(SHARED);
}

else if(currentLine->getFlags() == EXCLUSIVE){

currentLine -> setFlags(EXCLUSIVE);

}

}//read hit



}//end read

if(*op == 'w'){

writes++;
if(currentLine == NULL){//write miss
writeMisses++;
cacheLine * nextLine = fillLine(address);
nextLine->setFlags(MODIFIED);

if (single == 1){

cachetocache++;

}
BusRdX++;
for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "x";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->MESI(i, address, op1.c_str());


}// end bus read X 

}//write miss



else {//write hit

updateLRU(currentLine);

if(currentLine->getFlags() == MODIFIED){
currentLine -> setFlags(MODIFIED);
}


else if(currentLine->getFlags() == SHARED){

currentLine -> setFlags(MODIFIED);

for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "g";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->MESI(i, address, op1.c_str());


}// end bus read

}

else if(currentLine->getFlags() == EXCLUSIVE){

currentLine -> setFlags(MODIFIED);

}

}//write hit




}//endwrite



if(*op == 'b'){

if(currentLine == NULL){

}
else{

if(currentLine->getFlags() == MODIFIED){
currentLine -> setFlags(SHARED);
interventions++;

writeBack(address);
flushes++;
}


else if(currentLine->getFlags() == SHARED){

currentLine -> setFlags(SHARED);


}

else if(currentLine->getFlags() == EXCLUSIVE){

currentLine -> setFlags(SHARED);
interventions++;
}

}


}//end BusRd

if(*op == 'x'){

if(currentLine == NULL){

}
else{

if(currentLine->getFlags() == MODIFIED){
currentLine -> setFlags(INVALID);
invalidations++;

writeBack(address);
flushes++;
}


else if(currentLine->getFlags() == SHARED){

currentLine -> setFlags(INVALID);
invalidations++;


}

else if(currentLine->getFlags() == EXCLUSIVE){

currentLine -> setFlags(INVALID);
invalidations++;

}

}


}//end BusRdX

if(*op == 'g'){



if(currentLine == NULL){

}
else{

if(currentLine->getFlags() == MODIFIED){

}


else if(currentLine->getFlags() == SHARED){

currentLine -> setFlags(INVALID);
invalidations++;


}

else if(currentLine->getFlags() == EXCLUSIVE){

}

}


}//end Busupgrade


}// end MESI

void Cache::DRAGON(int current_processor,ulong address, const char* op){ // start DRAGON
current_proc = current_processor;
currentCycle++;
cacheLine *currentLine = findLine(address);
//cout <<"DRAGON CALLED"<<endl;
int single;

for(int i =0; i< procs; i++){

if(i == current_proc)
continue;

single = arrayOfCaches[i]->searchCache(address);
if(single == 1)
break;


}
if(*op == 'r'){

reads++;

if(currentLine == NULL){

readMisses++;
cacheLine *nextLine = fillLine(address);
if(single ==0){
nextLine -> setFlags(EXCLUSIVE);
}

if(single ==1){
nextLine -> setFlags(ShCl);
}

for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "b";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());


}// end bus read

}

else {

updateLRU(currentLine);

      if( currentLine->getFlags() == MODIFIED )
      {
         currentLine->setFlags(MODIFIED);   //Stay in modified.
      }  //MODIFIED end

      else if(currentLine->getFlags() == EXCLUSIVE)
      {
         currentLine->setFlags(EXCLUSIVE);
      }

      //SHARED MODIFIED state
      else if( currentLine->getFlags() == ShMod)
      {
         currentLine->setFlags(ShMod);
      }

      //SHARED CLEAN state
      else if ( currentLine->getFlags() == ShCl)
      {
         currentLine->setFlags(ShCl);
      } 


}

}//end read

if(*op == 'w'){
   
writes++;

if(currentLine == NULL){

writeMisses++;
cacheLine *nextLine = fillLine(address);
if(single ==0){
nextLine -> setFlags(MODIFIED);

for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "b";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());
}

if(single ==1){
nextLine -> setFlags(ShMod);

for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "b";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());
}

for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "u";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());
}
}




}// end bus read

}//end write miss

else{

updateLRU(currentLine);
      if( currentLine->getFlags() == MODIFIED )
      {
         currentLine->setFlags(MODIFIED);   //Stay in modified.
      }  //MODIFIED end

      else if(currentLine->getFlags() == EXCLUSIVE)
      {
         currentLine->setFlags(MODIFIED);
      }

      //SHARED MODIFIED state
      else if( currentLine->getFlags() == ShMod)
      {
         if(single ==0){
            currentLine->setFlags(MODIFIED);

            for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "u";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());
}


         }
         else if (single == 1){
            
            currentLine->setFlags(ShMod);

            for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "u";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());
}
            
            
         }
         
      }

      //SHARED CLEAN state
      else if ( currentLine->getFlags() == ShCl)
      {
         if(single ==0){
            currentLine->setFlags(MODIFIED);

            for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "u";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());
}


         }
         else if (single == 1){
            
            currentLine->setFlags(ShMod);

            for(int i=0; i<procs; i++){
//cout <<"issuing bus read line 83 CALLED"<<endl;
if(i == current_proc){
//cout <<"line 85"<<endl;
continue;
}

string op1 = "u";
//cout <<"line 90"<<endl;
arrayOfCaches[i]->DRAGON(i, address, op1.c_str());
}
            
            
         }
      } 



}



   

}//endwrite

if(*op == 'b'){

if(currentLine == NULL){


}

else {

      if( currentLine->getFlags() == MODIFIED )
      {
         currentLine->setFlags(ShMod);
         interventions++;
         
         flushes++;   //Stay in modified.
      }  //MODIFIED end

      else if(currentLine->getFlags() == EXCLUSIVE)
      {
         currentLine->setFlags(ShCl);
         interventions++;
      }

      //SHARED MODIFIED state
      else if( currentLine->getFlags() == ShMod)
      {
         currentLine->setFlags(ShMod);

         flushes++;
      }

      //SHARED CLEAN state
      else if ( currentLine->getFlags() == ShCl)
      {
         currentLine->setFlags(ShCl);
      } 


}

}//end BusRd

if(*op == 'u'){
if(currentLine == NULL){


}

else {

      if( currentLine->getFlags() == MODIFIED )
      {
         
      }  //MODIFIED end

      else if(currentLine->getFlags() == EXCLUSIVE)
      {
         
      }

      //SHARED MODIFIED state
      else if( currentLine->getFlags() == ShMod)
      {
         currentLine->setFlags(ShCl);
         ulong temp_tag = calcTag(address);
         currentLine -> setTag(temp_tag);
      }

      //SHARED CLEAN state
      else if ( currentLine->getFlags() == ShCl)
      {
         currentLine->setFlags(ShCl);
         ulong temp_tag = calcTag(address);
         currentLine -> setTag(temp_tag);
      } 


}



}//end bus update

}// end DRAGON


//void Cache::Access(ulong addr,uchar op)
//{
//	currentCycle++;/*per cache global counter to maintain LRU order 
//			among cache ways, updated on every cache access*/
//       	
//	if(op == 'w') writes++;
//	else          reads++;
//	
//	cacheLine * line = findLine(addr);
//	if(line == NULL)/*miss*/
//	{
//		if(op == 'w') writeMisses++;
//		else readMisses++;
//
//		cacheLine *newline = fillLine(addr);
//  		if(op == 'w') newline->setFlags(DIRTY);    
//		
//	}
//	else
//	{
//		/**since it's a hit, update LRU and update dirty flag**/
//		updateLRU(line);
//		if(op == 'w') line->setFlags(DIRTY);
//	}
//}



int Cache::searchCache(ulong address){

int found =0;
ulong temp_tag, temp_index;

temp_tag = calcTag(address);
temp_index = calcIndex(address);

for(uint i=0; i<assoc; i++){

if(cache[temp_index][i].isValid() && cache[temp_index][i].getTag() == temp_tag){
found =1;
}//end if

}// end for


return found;
}


/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == MODIFIED || victim->getFlags() == ShMod ) writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   //victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats(ulong memRead)
{ 	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
	//cout<<"===== Simulation results      =====" <<endl;

float missrate = (float)(readMisses+writeMisses)/(reads+writes);
missrate = missrate * 100;

cout<< "01. number of reads:		               	"<<reads<<endl;
cout<< "02. number of read misses:	            	"<<readMisses<<endl;
cout<< "03. number of writes:			       "<<writes<<endl;
cout<< "04. number of write misses:	            	"<<writeMisses<<endl;
    cout.precision(2);
    cout.unsetf(ios::floatfield);
    cout.setf(ios::fixed, ios::floatfield);
    cout<< "05. total miss rate		                "<<missrate<<"%"<<endl;
cout.unsetf(ios::floatfield);

cout<< "06. number of writebacks:		    "<<writeBacks<<endl;
cout<< "07. number of cache-to-cache transfers:         "<<cachetocache<<endl;
cout<< "08. number of memory transactions:               "<<memRead<<endl;
cout<< "09. number of interventions:	         	"<<interventions<<endl;
cout<< "10. number of invalidations:	         	"<<invalidations<<endl;
cout<< "11. number of flushes:		            	"<<flushes<<endl;
cout<< "12. number of BusRdX:		                  "<<BusRdX<<endl;

}
