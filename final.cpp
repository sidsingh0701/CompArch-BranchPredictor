#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <bitset>
#include <iomanip>
#include <stdio.h>



using namespace std;

// Print Initializer

void startprint(int blocksize,int totalsize, int assoc, int repolicy, int writepolicy, string name){
  cout << "  ===== Simulator configuration =====" << endl;
  cout << "  L1_BLOCKSIZE:                    " << blocksize << endl;
  cout << "  L1_SIZE:                         " << totalsize << endl;
  cout << "  L1_ASSOC:                         " << assoc << endl;
  cout << "  L1_REPLACEMENT_POLICY:            " << repolicy << endl;
  cout << "  L1_WRITE_POLICY:                  " << writepolicy << endl;
  cout << "  trace_file:           " << name << endl;
  cout << "  ===================================" << endl;
  cout << endl;
  cout << "===== L1 contents =====" << endl;
}

/* Function for converting Decimal to Binary String*/

string DecToBin2(int number)
{
    string result = "";

    do
    {
        if ( (number & 1) == 0 )
            result += "0";
        else
            result += "1";

        number >>= 1;
    } while ( number );

    reverse(result.begin(), result.end());
    return result;
}

/* Splitting Address in Tag, Index and Block Offset Bits*/

void splitaddress(string mainbits,string& tagbits,string& setbits,string& blockbits,int tag,int sets,int blksize){
	blockbits = mainbits.substr((tag+sets),blksize);
	setbits = mainbits.substr(tag,sets);
	tagbits = mainbits.substr(0,tag);
}



class CACHE
{
private:
	int read;
	int rorob;
	int rorog;
	int roroh;
	int read_miss;
	int write;
	int write_miss;
	int missrate;
	int numwb;
	int memtraffic;
	int totpredict;
	int misspredict;
	int bdummy,gdummy;
	double average;
	CACHE* nextlevel;
	string *tagbits;
	int **blockcount;
	int *setcount;
	int **validbit;
	int **dbit;
	int **lfucount;
	int *pb_table; // Bimodal Branch Predictor
	int *pg_table;
	char bpredict,gpredict;	
	string gbh;
	int gbnumber;
	int *hybrid_table; //Hybrid Predictor

public:
	CACHE (int K,int m1,int m2,int n,int tag,int set_row,int sets, int blksize,int assoc);
	void readFromAddress(string mainbits,int tag,int sets,int blksize,int repolicy,int assoc);
	void writeToAddress(string mainbits,int tag,int sets,int blksize,int writepolicy,int repolicy,int assoc);
	void checkprint(int sets,int assoc,int frame,int writepolicy, int totalsize, int blocksize);
	void bimodal(char a,int index);
	void gshare(char a,string index,int n);
	void bimodal_print();
	void gshare_print();
	void hybrid_print();
	void updatebn(char bpredict,char a,int index);
	void updatebt(char bpredict,char a,int index);
	void updategn(char gpredict,char a,int finalindex);
	void updategt(char gpredict,char a,int finalindex);	
	void hybrid(char a,int grow_info,int brow_info ,int row_info);
	void hbimodal(char a, int index);
	int hgshare(char a, string index,int n);
//	void update(); 
};

void CACHE::hybrid_print(){
	//cout << totpredict << " " << misspredict << endl;
	int i;	
	for(i=0;i<roroh;i++){
	   cout << i << "  	"  << hybrid_table[i] << endl;
	}
}

void CACHE::bimodal_print(){
	cout << totpredict << " " << misspredict << endl;
	int i;	
	for(i=0;i<rorob;i++){
	  // cout << i << "	"  << pb_table[i] << endl;
	}
}

void CACHE::gshare_print(){
	cout << totpredict << " " << misspredict << endl;
	int i;	
	for(i=0;i<rorog;i++){
	  //cout << i << "	"  << pg_table[i] << endl;
	}
}

void CACHE::updatebn(char bpredict,char a,int index){		
		if(bpredict != a){
			//misspredict++;
			pb_table[index]++;
		}
		else{
		     if(pb_table[index] == 1)			
			pb_table[index]--;
		}
		//cout << pb_table[index] << "here in bn" << endl;
}

void CACHE::updatebt(char bpredict,char a,int index){
		if(bpredict != a){
			//misspredict++;
			pb_table[index]--;
		}
		else{
			if(pb_table[index] == 2)
				pb_table[index]++;
		}
		//cout << pb_table[index] << " here in bt" << endl;
}

void CACHE::updategn(char gpredict,char a,int finalindex){
		if(gpredict != a){
			//misspredict++;
			pg_table[finalindex]++;
		}
		else{
		     if(pg_table[finalindex] == 1)			
			pg_table[finalindex]--;
		}
}

void CACHE::updategt(char gpredict,char a,int finalindex){
		if(gpredict != a){
			//misspredict++;
			pg_table[finalindex]--;
		}
		else{
			if(pg_table[finalindex] == 2)
				pg_table[finalindex]++;
		}
}

void CACHE::hbimodal(char a,int index){
	bdummy = pb_table[index];
	//totpredict++;
	//char predict;
	if(bdummy <= 1){
		bpredict = 'n';
		//updatebn(bpredict,a,index);
	}
	else{
		bpredict = 't';
		//updatebt(bpredict,a,index);
	}
	//cout << misspredict << " " << predict << " " << a << " " << predict_table[index] << " " << index << endl;
	//cout << "Logging out" << endl;
}

int CACHE::hgshare(char a,string index,int n){
	//totpredict++;
	//cout << gbnumber << endl;
	string dummy,dummy2;
	int value;
	int finalindex;	
	char predict;	
	//cout << index << " " << a << " " << n << endl;
	string check1;	
	dummy = index.substr(0,n);
	//cout << dummy << endl;
	value = strtoull(dummy.c_str(),NULL,2);
	//cout << value << endl;
	//cout << value << " " << dummy << endl;
	//cout << gbnumber << endl;	
	finalindex = gbnumber ^ value;
	//cout << finalindex << endl;
		//cout << "Here" << endl;
	check1 = DecToBin2(finalindex);
	//cout << check1 << endl;
	//cout << "Here" << endl;
	int upper_len = n-check1.length();
		//cout << "Here" << endl;		
	while(upper_len){                                               //INFINITE LOOP HERE
			check1.insert(0,"0");
		//		cout << "Here" << endl;
			upper_len--;
	}
		//cout << "Here" << endl;
		
	dummy2 = index.substr(n,(24-n));
	//cout << check1 << endl;
	//cout << index << endl;
	dummy = check1 + dummy2;
	//cout << dummy << endl;
	finalindex = strtoull(dummy.c_str(),NULL,2);
	//cout << finalindex << endl;
	//cout << finalindex << endl;
	int tableval;
	gdummy = pg_table[finalindex];
	if(gdummy <= 1){
		gpredict = 'n';
		/*if(gpredict != a){
			misspredict++;
			pg_table[finalindex]++;
		}
		else{
		     if(pg_table[finalindex] == 1)			
			pg_table[finalindex]--;
		}*/
		if(a == 't'){
			gbh.insert(0,"1");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
		else{
			gbh.insert(0,"0");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
	}
	else{
		gpredict = 't';
		/*if(gpredict != a){
			misspredict++;
			pg_table[finalindex]--;
		}
		else{
			if(pg_table[finalindex] == 2)
				pg_table[finalindex]++;
		}*/
		if(a == 't'){
			gbh.insert(0,"1");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
		else{
			gbh.insert(0,"0");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
	}
	return finalindex;
	//cout << misspredict << " " << predict << " " << a << " " << predict_table[index] << " " << index << endl;
}

void CACHE::hybrid(char a,int grow_info,int brow_info,int row_info){
	totpredict++;	
	int check = hybrid_table[row_info];
	int dummy,dummy2;
	int flagb = 1,flagg=1;
	//cout << check << "in hybrid func" << endl;
	if(a != bpredict){
			//misspredict++;
			flagb = 0;
	}
	if(a != gpredict){
			//misspredict++;
			flagg = 0;
	} 
	//cout << flagb << "&&" << flagg << endl;
	if(check <= 1){
		//Bimodal
		if(a != bpredict){
			misspredict++;
		}
		//cout << brow_info << endl;
		dummy = pb_table[brow_info];
		//cout << dummy << endl;
		if(dummy <= 1){
			updatebn(bpredict,a,brow_info);
		}
		else{
			updatebt(bpredict,a,brow_info);
		}
	}
	else{
		if(a != gpredict){
			misspredict++;
		} 
		dummy2 = pg_table[grow_info];
		if(dummy2 <= 1){
			updategn(gpredict,a,grow_info);
		}
		else{
			updategt(gpredict,a,grow_info);
		}
		//G Share
	}
	int upgr = hybrid_table[row_info];
	//cout << flagg << " " << flagb << " " << upgr << endl;
	if((flagb == 1) && (flagg == 0)){
		if((upgr == 1) || (upgr == 2) || (upgr == 3)){
			hybrid_table[row_info]--;
			//cout << "Hello1" << endl;		
		}	
	}
	if((flagb == 0) && (flagg == 1)){
		if((upgr == 0) || (upgr == 1) || (upgr == 2)){
			hybrid_table[row_info]++;
			//cout << "Hello2" << endl;
		}
	}
	//cout << hybrid_table[row_info] << "** from here in hy" << endl;
	/*cout << "CHOOSER index: " << row_info << " old value: " << upgr << " new value " << hybrid_table[row_info] << endl;
	if(check <= 1){
		cout << "BIMODAL index: " << brow_info << " old value: " << dummy << " new value " << pb_table[brow_info] << endl;
		//cout << "GSHARE index: " << grow_info <<  " old value: " << flagg << " new value " << flagb << endl;
		cout << "BHR UPDATED: " << gbnumber << endl;
	}
	else{
		cout << "GSHARE index: " << grow_info <<  " old value: " << dummy2 << " new value " << pg_table[grow_info] << endl;
		cout << "BHR UPDATED: " << gbnumber << endl;
	}*/
}


void CACHE::bimodal(char a,int index){
	int dummy = pb_table[index];
	totpredict++;
	//char predict;
	if(dummy <= 1){
		bpredict = 'n';
		if(bpredict != a){
			misspredict++;
			pb_table[index]++;
		}
		else{
		     if(pb_table[index] == 1)			
			pb_table[index]--;
		}
	}
	else{
		bpredict = 't';
		if(bpredict != a){
			misspredict++;
			pb_table[index]--;
		}
		else{
			if(pb_table[index] == 2)
				pb_table[index]++;
		}
	}
	//cout << misspredict << " " << predict << " " << a << " " << predict_table[index] << " " << index << endl;
}

void CACHE::gshare(char a,string index,int n){
	totpredict++;
	string dummy,dummy2;
	int value;
	int finalindex;	
	char predict;	
	string check1;	
	//cout << "Here" << endl;
	dummy = index.substr(0,n);
	value = strtoull(dummy.c_str(),NULL,2);
	//cout << value << " " << dummy << endl;	
	finalindex = gbnumber ^ value;
	check1 = DecToBin2(finalindex);
	int upper_len = n-check1.length();
		//cout << "Here" << endl;		
	while(upper_len){
			check1.insert(0,"0");
			upper_len--;
	}
	dummy2 = index.substr(n,(24-n));
	//cout << check1 << endl;
	//cout << dummy2 << endl;
	dummy = check1 + dummy2;
	//cout << dummy << endl;
	finalindex = strtoull(dummy.c_str(),NULL,2);
	//cout << finalindex << endl;
	int tableval;
	tableval = pg_table[finalindex];
		//cout << "Here" << endl;
	if(tableval <= 1){
		gpredict = 'n';
		if(gpredict != a){
			misspredict++;
			pg_table[finalindex]++;
		}
		else{
		     if(pg_table[finalindex] == 1)			
			pg_table[finalindex]--;
		}
		if(a == 't'){
			gbh.insert(0,"1");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
		else{
			gbh.insert(0,"0");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
	}
	else{
		gpredict = 't';
		if(gpredict != a){
			misspredict++;
			pg_table[finalindex]--;
		}
		else{
			if(pg_table[finalindex] == 2)
				pg_table[finalindex]++;
		}
		if(a == 't'){
			gbh.insert(0,"1");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
		else{
			gbh.insert(0,"0");
			gbh = gbh.substr(0,n);
			gbnumber = strtoull(gbh.c_str(),NULL,2);
		}
	}
	//cout << misspredict << " " << predict << " " << a << " " << predict_table[index] << " " << index << endl;
}

void CACHE::checkprint(int sets,int assoc,int frame,int writepolicy,int totalsize,int blocksize){
	double miss_penalty = (double)(20 + (double)(0.5*(double)(blocksize/16)));
	double hit_time = (double)(0.25 + (double)(2.5 * (double)(totalsize/(double)(512*1024))) + (double)(0.025*assoc) + (double)(0.025*(double)(blocksize/16))); 
	double miss_rate = ((double)(read_miss + write_miss)/(double)(read + write));
	average = (double)(hit_time + (double)(miss_penalty * miss_rate));
	int dummy,j=0,count=0;
	//cout << miss_rate << " MR"<< hit_time << "HT" << miss_penalty << "MP" << average << endl;
	if(writepolicy == 0){
		memtraffic = read_miss + write_miss + numwb;
	}
	else{
		memtraffic = read_miss + write;

	}
	string checker;
	//cout << average << endl; 	
	for(int i=0;i<sets;i++){
		count = 0;	
		//cout.width(4);
		cout << "set  " << i << ":   ";
		if(assoc == 1){
			dummy = strtol(tagbits[i].c_str(),0,2);
			//cout << "set " << i << ": "; 
			printf("%X",dummy);
			cout << " ";
			if(dbit[i][(assoc-1)])
				cout << "D" << endl;
			else
				cout << " " << endl;
		}	
		else{
			for(j=0;j<assoc;j++){	
				checker = tagbits[i].substr(count,frame);			
				dummy = strtol(checker.c_str(),0,2);
				count = count+frame;
				printf("%X ",dummy); 				
				if(dbit[i][j])
					cout << "D   ";
				else
				cout << "     ";	
				
			}
			cout << endl;
		}			
	}

  cout << endl;	
  cout << "  ====== Simulation results (raw) ======" << endl;
  cout << "  a. number of L1 reads:           " << read << endl;
  cout << "  b. number of L1 read misses:      " << read_miss << endl;
  cout << "  c. number of L1 writes:          " << write << endl;
  cout << "  d. number of L1 write misses:     " << write_miss << endl;
  cout << "  e. L1 miss rate:                " << setprecision(4) << fixed << miss_rate << endl;
  cout << "  f. number of writebacks from L1:  " << numwb << endl;
  cout << "  g. total memory traffic:         " << memtraffic << endl;
  cout << endl;
  cout << "  ==== Simulation results (performance) ====" << endl;
  cout << "  1. average access time:         " << setprecision(4) << fixed << average << " ns" << endl;
}

/* CACHE Constructor */

CACHE::CACHE (int K,int m1,int m2,int n,int tag, int set_row,int sets, int blksize, int assoc){	
	int i,j;
	totpredict = 0;
	misspredict = 0;
	int k = 2 << (m2-1);
	int k2 = 2 << (m1-1);
	int k3 = 2 << (K-1);
	rorob = k;
	rorog = k2;
	roroh = k3;
	//cout << roroh << endl;
	gbnumber = 0;
	//cout << k << endl;
	//gbh = new string;	
	for(i=0;i<n;i++)
		gbh.push_back('0');
	pb_table = new int [k];
	for(i=0;i<k;i++){
		pb_table[i] = 2;
	}
	pg_table = new int [k2];
	for(i=0;i<k2;i++){
		pg_table[i] = 2;
	}
	hybrid_table = new int [k3];
	for(i=0;i<k3;i++){
		hybrid_table[i] = 1;
	}
	//cout << hybrid_table[510] << endl;
	//cout << k << " " << m2 << endl;
	read = 0;
	read_miss = 0;
	average = 0.0;
	write = 0;
	write_miss = 0;
	missrate = 0;
	memtraffic = 0;
	numwb = 0;
	nextlevel = NULL;
	tagbits = new string [set_row];
	string dummy,temp;
	for(i=0;i<tag;i++)
		dummy.push_back('0');
	for(i=0;i<set_row;i++){
		temp = "";
		for(j=0;j<assoc;j++){
			temp.append(dummy);
		}
		tagbits[i].append(temp);
	}
	setcount = new int [set_row];
	for(i=0;i<set_row;i++){
		setcount[i] = 0;
	}
	lfucount = new int* [set_row];
	for(i=0;i<set_row;i++){
		lfucount[i] = new int [assoc]; 
	}	
	for(i=0;i<set_row;i++){
		for(j=0;j<assoc;j++){
			lfucount[i][j] = 0;
		}
	}
	validbit = new int* [set_row];
	for(i=0;i<set_row;i++){
		validbit[i] = new int [assoc]; 
	}	
	for(i=0;i<set_row;i++){
		for(j=0;j<assoc;j++){
			validbit[i][j] = 0;
		}
	}
	blockcount = new int* [set_row];
	for(i=0;i<set_row;i++){
		blockcount[i] = new int [assoc]; 
	}
	for(i=0;i<set_row;i++){
		for(j=0;j<assoc;j++){
			blockcount[i][j] = (assoc-j-1);
		}
	}	
	dbit = new int* [set_row];
	for(i=0;i<set_row;i++){
		dbit[i] = new int [assoc];
	}	
	for(i=0;i<set_row;i++){
		for(j=0;j<assoc;j++){
			dbit[i][j] = 0;
		}
	}
	
}


/*Read Function of Class handles LRU and LFU*/
//HAVE TO ADD VARIABLE G from Specs
// DO GENERALIZE FOR REPOLICY ND SET UP VALID BITS REMOVE BLOCKCOUNT ND CHECK AGAIN


void CACHE::readFromAddress(string mainbits,int tag,int sets, int blksize, int repolicy,int assoc){
	read ++;
	string setbits,tgbits,blockbits;
	splitaddress(mainbits,tgbits,setbits,blockbits,tag,sets,blksize);
	//cout << tgbits << endl;	
	int row_num;
	row_num = strtoull(setbits.c_str(),NULL,2);
	if(repolicy == 0){
		if(assoc == 1){
			if((tgbits.compare(tagbits[row_num]) == 0) && (validbit[row_num][(assoc-1)] == 1)){
				//cout << "hit" << endl;
			}
			else if(validbit[row_num][(assoc-1)] == 0){
				read_miss ++; 
				tagbits[row_num] = tgbits;
				validbit[row_num][(assoc-1)] = 1;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 0;				
				
			}
			else if(tgbits.compare(tagbits[row_num]) != 0){
				read_miss++;
				tagbits[row_num] = tgbits;
				validbit[row_num][(assoc-1)] = 1;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 0;			
			}
		}
		else{
			int i=0,j,index=0,hit=-1,count=0,max = -100;
			string dummy;
			int point=0;
			bool flag = false;
			for(i=0;i<assoc;i++){
				dummy = tagbits[row_num].substr(index,tag);
				hit = dummy.compare(tgbits);
				if(hit == 0){
					break;	
				}
				count++;
				index = index+tag;
			}	
			if((hit == 0) && (validbit[row_num][count]==1)){
				int temp = blockcount[row_num][count];	
				for(i=0;i<assoc;i++){
					if(blockcount[row_num][i] < temp){
						blockcount[row_num][i] += 1;
					}
				}
				blockcount[row_num][count] = 0;
			}
			else {				
				for(i=0;i<assoc;i++){
					if(validbit[row_num][i] == 0){
						flag = true;
						point = i;
						break;
					}
				}
				if(flag == false){	
						read_miss++;							
						for(i=0;i<assoc;i++){
							if(max < blockcount[row_num][i]){
								max = blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<assoc;i++){
							if(i == count){
								blockcount[row_num][i] = 0;
							}
							else{
								blockcount[row_num][i] ++;
							}
						}
						//cout << "Zone 1" << endl;
						if(dbit[row_num][count]==1){ //Replace Here for Early Matching 2 nd index (assoc-1)
							numwb++;
						}
						dbit[row_num][count] = 0;
						int start = count*tag;
						tagbits[row_num].replace(start,tag,tgbits);
				}	
				else{
						read_miss++;						
						for(i=0;i<assoc;i++){
							if(max < blockcount[row_num][i]){
								max = blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<assoc;i++){
							if(i == count){
								blockcount[row_num][i] = 0;
							}
							else{
								blockcount[row_num][i] ++;
							}
						}					
						validbit[row_num][count] = 1;
						if(dbit[row_num][count]==1){ //SAME HERE
							numwb++;
						}
						dbit[row_num][count] = 0;
						int begin = point*tag;
						tagbits[row_num].replace(begin,tag,tgbits);
						//cout << "INVALID ZONE" << endl;
				}
			}
		}
	}
	else if(repolicy == 1){
		if(assoc == 1){
			if((tgbits.compare(tagbits[row_num]) == 0) && (validbit[row_num][(assoc-1)] == 1)){
				lfucount[row_num][(assoc-1)] ++;
			}
			else if(validbit[row_num][(assoc-1)] == 0){
				read_miss ++; 
				tagbits[row_num] = tgbits;
				validbit[row_num][(assoc-1)] = 1;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 0;
				setcount[row_num] = lfucount[row_num][(assoc-1)];
				lfucount[row_num][(assoc-1)] = setcount[row_num] + 1;			
			}
			else if(tgbits.compare(tagbits[row_num]) != 0){
				read_miss++;
				tagbits[row_num] = tgbits;
				validbit[row_num][(assoc-1)] = 1;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 0;
				setcount[row_num] = lfucount[row_num][(assoc-1)];
				lfucount[row_num][(assoc-1)] = setcount[row_num] + 1;			
			}
		}
		else{
			int i=0,j,index=0,hit=-1,count=0,min = 1000000;
			string dummy;
			int point=0;
			bool flag = false;
			for(i=0;i<assoc;i++){
				dummy = tagbits[row_num].substr(index,tag);
				hit = dummy.compare(tgbits);
				if(hit == 0){
					break;	
				}
				count++;
				index = index+tag;
			}	
			if(hit == 0){
				lfucount[row_num][count] ++;
				//cout << "Read Hit" << endl;
			}
			else {				
					read_miss++;
					//cout << "Read Miss" << endl;													
					for(i=0;i<assoc;i++){
						if(min > lfucount[row_num][i]){
							min = lfucount[row_num][i];
						}
					}
					for(i=0;i<assoc;i++){
						if(min == lfucount[row_num][i]){
							count = i;
							break;
						}
					}
					setcount[row_num] = lfucount[row_num][count];
					lfucount[row_num][count] = setcount[row_num] + 1; 
					validbit[row_num][count] = 1;
					if(dbit[row_num][count]==1){
						numwb++;
						dbit[row_num][count] = 0;
					}					
					int start = count*tag;
					tagbits[row_num].replace(start,tag,tgbits);
				}
			}
		}	
}

void CACHE::writeToAddress(string mainbits,int tag,int sets,int blksize,int writepolicy,int repolicy,int assoc){
	write++;
	string setbits,tgbits,blockbits;
	int hexa;
	splitaddress(mainbits,tgbits,setbits,blockbits,tag,sets,blksize);
	int row_num;
	row_num = strtoull(setbits.c_str(),NULL,2);
	if(repolicy == 0){
		if(writepolicy == 0){
		 if(assoc == 1){
			if((tgbits.compare(tagbits[row_num]) == 0) && (validbit[row_num][(assoc-1)] == 1)){
				dbit[row_num][(assoc-1)] = 1;
			}
			else if(validbit[row_num][(assoc-1)] == 0){
				write_miss++;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 1;
				tagbits[row_num] = tgbits;
				validbit[row_num][(assoc-1)] = 1;
				// HAVE TO SETUP WRITE BACK UPDATE HERE TOO DEPENDING UPON WRITE POLICY
			}
			else if(tgbits.compare(tagbits[row_num]) != 0){
				write_miss++;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 1;
				tagbits[row_num] = tgbits;
				//cout << tagbits[row_num] << endl;
				validbit[row_num][(assoc-1)] = 1;
			}
		}
		else{
			int i=0,j,index=0,hit=-1,count=0,max = -100;
			string dummy;
			int point=0;
			bool flag = false;
			for(i=0;i<assoc;i++){
				dummy = tagbits[row_num].substr(index,tag);
				hit = dummy.compare(tgbits);
				if(hit == 0){
					break;	
				}
				count++;
				index = index+tag;
			}	
			if((hit == 0) && (validbit[row_num][count]==1)){
				int temp = blockcount[row_num][count];	
				for(i=0;i<assoc;i++){
					if(blockcount[row_num][i] < temp){
						blockcount[row_num][i] += 1;
					}
				}
				blockcount[row_num][count] = 0;
				dbit[row_num][count] = 1;
			}
			else {				
				for(i=0;i<assoc;i++){
					if(validbit[row_num][i] == 0){
						flag = true;
						point = i;
						break;
					}
				}
				if(flag == false){	
						write_miss++;							
						for(i=0;i<assoc;i++){
							if(max < blockcount[row_num][i]){
								max = blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<assoc;i++){
							if(i == count){
								blockcount[row_num][i] = 0;
							}
							else{
								blockcount[row_num][i] ++;
							}
						}
						//cout << "Zone 1" << endl;
						if(dbit[row_num][count]==1){ //DOG TRAILS
							numwb++;
						}
						dbit[row_num][count] = 1;
						int start = count*tag;
						tagbits[row_num].replace(start,tag,tgbits);
				}	
				else{
						write_miss++;						
						for(i=0;i<assoc;i++){
							if(max < blockcount[row_num][i]){
								max = blockcount[row_num][i];
								count = i;
							}
						}
						for(i=0;i<assoc;i++){
							if(i == count){
								blockcount[row_num][i] = 0;
							}
							else{
								blockcount[row_num][i] ++;
							}
						}					
						validbit[row_num][count] = 1;
						if(dbit[row_num][count]==1){ //DOG TRAILS
							numwb++;
						}
						dbit[row_num][count] = 1;
						int begin = point*tag;
						tagbits[row_num].replace(begin,tag,tgbits);
						//cout << "INVALID ZONE" << endl;
				}
			}
		}
	}
	else if(writepolicy == 1){		
		int i=0,j,index=0,hit=-1,count=0,max = -100;
		string dummy;
		int point=0;
		bool flag = false;
		for(i=0;i<assoc;i++){
			dummy = tagbits[row_num].substr(index,tag);
			hit = dummy.compare(tgbits);
			if(hit == 0){
				break;	
			}
			count++;
			index = index+tag;
		}
		if((hit == 0) && (validbit[row_num][count] == 1)){
			int temp = blockcount[row_num][count];	
			for(i=0;i<assoc;i++){
				if(blockcount[row_num][i] < temp){
					blockcount[row_num][i] += 1;
				}
			}
			blockcount[row_num][count] = 0;
		}
		else{
			write_miss++;
		}
	}
      }
      else if(repolicy == 1){
		//WRITE POLICIES FOR SECOND KIND OF UPDATE
	if(writepolicy == 1){		
		int i=0,j,index=0,hit=-1,count=0,max = -100;
		string dummy;
		int point=0;
		bool flag = false;
		for(i=0;i<assoc;i++){
			dummy = tagbits[row_num].substr(index,tag);
			hit = dummy.compare(tgbits);
			if(hit == 0){
				break;	
			}
			count++;
			index = index+tag;
		}
		if(hit == 0){
			lfucount[row_num][count] ++;
			//cout << "Write Hit" << endl;
		}
		else{
			write_miss++;
			//cout << "Write Miss" << endl;
			
		}
	}
	else if(writepolicy == 0){
		if(assoc == 1){
			if((tgbits.compare(tagbits[row_num]) == 0) && (validbit[row_num][(assoc-1)] == 1)){
				lfucount[row_num][(assoc-1)] ++;
			}
			else if(validbit[row_num][(assoc-1)] == 0){
				read_miss ++; 
				tagbits[row_num] = tgbits;
				validbit[row_num][(assoc-1)] = 1;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 0;
				setcount[row_num] = lfucount[row_num][(assoc-1)];
				lfucount[row_num][(assoc-1)] = setcount[row_num] + 1;			
			}
			else if(tgbits.compare(tagbits[row_num]) != 0){
				read_miss++;
				tagbits[row_num] = tgbits;
				validbit[row_num][(assoc-1)] = 1;
				if(dbit[row_num][(assoc-1)]==1){
					numwb++;
				}
				dbit[row_num][(assoc-1)] = 0;
				setcount[row_num] = lfucount[row_num][(assoc-1)];
				lfucount[row_num][(assoc-1)] = setcount[row_num] + 1;			
			}
		}
		else{
			int i=0,j,index=0,hit=-1,count=0,min = 1000000;
			string dummy;
			int point=0;
			bool flag = false;
			for(i=0;i<assoc;i++){
				dummy = tagbits[row_num].substr(index,tag);
				hit = dummy.compare(tgbits);
				if(hit == 0){
					break;	
				}
				count++;
				index = index+tag;
			}	
			if(hit == 0){
				lfucount[row_num][count] ++;
				dbit[row_num][count] = 1;
				//cout << "Read Hit" << endl;
			}
			else {				
					count = 0;					
					write_miss++;
					//cout << "Write Miss" << endl;													
					for(i=0;i<assoc;i++){
						if(min > lfucount[row_num][i]){
							min = lfucount[row_num][i];
						}
					}
					for(i=0;i<assoc;i++){
						if(min == lfucount[row_num][i]){
							count = i;
							break;
						}
					}
					setcount[row_num] = lfucount[row_num][count];
					lfucount[row_num][count] = setcount[row_num] + 1; 
					//validbit[row_num][count] = 1;
					if(dbit[row_num][count]==1){
						numwb++;
					}	
					dbit[row_num][count] = 1;				
					int start = count*tag;
					tagbits[row_num].replace(start,tag,tgbits);
				}
			}
		}
 	}
}


int main(int argc,char *argv[]){
	/*int blocksize,totalsize,assoc,repolicy,writepolicy;
	string binst[100000];
	int address[100000];	
	blocksize = atoi(argv[1]);
	totalsize = atoi(argv[2]);
	assoc = atoi(argv[3]);
	repolicy = atoi(argv[4]);
	writepolicy = atoi(argv[5]);
	ifstream infile(argv[6]);
	double sets,tag,index;
	string filename = argv[6];
	//cout << infile << endl;
	char dummy,a[100000];
	string dumb,b[100000];
	string parse = "0";
	int i=0;
	int value;
	int address_len;
	int checker; //temporary debug variable
	string setbits,tagbits,blockbits;*/
	string orig,orig2;
	string s1 = "bimodal", s2 = "gshare", s3 = "hybrid";
	int flag1 =0,flag2 =0;		
	char dummy;
	int i;
	int K=0;
	string dumb;
	string binst;
	string parse = "0";
	int m2=0,m1=0;
	int btb_size;
	int btb_assoc;
	string filename;
	char a;
	int n=0;
	int address_len;
	int value;
	int row_info;
	/*if(argv[5] == NULL){
		int address_len;
		int value;
		int row_info;
	//cout << m2 << " " << btb_size << " " << btb_assoc << " " << filename << " " << endl;
	//if(argc == 6){	
	 	m2 = atoi(argv[1]);
	 	btb_size = atoi(argv[2]);
	 	btb_assoc = atoi(argv[3]);
	 	ifstream infile(argv[4]);
		 filename = argv[4];
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		while(infile >> dumb >> dummy){
			a = dummy;	
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			binst = binst.substr((22-m2),m2);
	        	row_info = strtoull(binst.c_str(),NULL,2);
			cachetrial.bimodal(a,row_info);
			i++;
		}
		cachetrial.bimodal_print();
		return 0;
	}
	if(argv[6]==NULL){
		int address_len;
		int value;
		int row_info;	
		 m1 = atoi(argv[1]);
		 n = atoi(argv[2]);
		 btb_size = atoi(argv[3]);
		 btb_assoc = atoi(argv[4]);
	 	ifstream infile(argv[5]);
	 	filename = argv[6];
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		while(infile >> dumb >> dummy){
			a = dummy;	
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			binst = binst.substr((22-m1),m1);
	        	row_info = strtoull(binst.c_str(),NULL,2);
			cachetrial.gshare(a,binst,n);
			//cachetrial.bimodal(a,row_info);
			//break;
			i++;
		}
		cachetrial.gshare_print();
		return 0;
	}*/
	/*	int address_len;
		int value;
		int row_info;
		int brow_info;
		int grow_info;
		 K = atoi(argv[1]);
		 m1 = atoi(argv[2]);
		 n = atoi(argv[3]);
		 m2 = atoi(argv[4]);
		 btb_size = atoi(argv[5]);
		 btb_assoc = atoi(argv[6]);
	 	ifstream infile(argv[7]);
	 	filename = argv[7];
		//cout << filename << endl;
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		while(infile >> dumb >> dummy){
			a = dummy;	
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			//Change from here 
			orig = binst;
			orig2 = binst;
			cout << orig2 << endl;
			orig2 = orig2.substr((22-m1),m1);
			cout << orig2 << endl;
			//cachetrial.gshare(a,orig2,n);
			//orig = orig.substr((22-m2),m2);
	        	//brow_info = strtoull(orig.c_str(),NULL,2);
			//cachetrial.hbimodal(a,brow_info);
			//binst = binst.substr((22-K),K);
			//cout << binst << endl;
	        	//row_info = strtoull(binst.c_str(),NULL,2);
			//cachetrial.hybrid(a,grow_info,brow_info,row_info);
			//cout << row_info << endl;
			//cachetrial.bimodal(a,row_info);
			break;
			i++;
		}
		//cachetrial.gshare_print();*/
		//cout << argc << endl;
	/*  if(argv[5] == NULL){
		int address_len;
		int value;
		int row_info;
	//cout << m2 << " " << btb_size << " " << btb_assoc << " " << filename << " " << endl;
	//if(argc == 6){	
	 	m2 = atoi(argv[1]);
	 	btb_size = atoi(argv[2]);
	 	btb_assoc = atoi(argv[3]);
	 	ifstream infile(argv[4]);
		 filename = argv[4];
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		while(infile >> dumb >> dummy){
			a = dummy;	
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			binst = binst.substr((22-m2),m2);
	        	row_info = strtoull(binst.c_str(),NULL,2);
			cachetrial.bimodal(a,row_info);
			i++;
		}
		cachetrial.bimodal_print();
		return 0;
	}
	else{
		int address_len;
		int value;
		int row_info;
		if(argc == 6){	
		 m1 = atoi(argv[1]);
		 n = atoi(argv[2]);
		 btb_size = atoi(argv[3]);
		 btb_assoc = atoi(argv[4]);
	 	ifstream infile(argv[5]);
	 	filename = argv[5];
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		while(infile >> dumb >> dummy){
			a = dummy;	
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			binst = binst.substr((22-m1),m1);
	        	row_info = strtoull(binst.c_str(),NULL,2);
			cachetrial.gshare(a,binst,n);
			//cachetrial.bimodal(a,row_info);
			//break;
			i++;
		}
		cachetrial.gshare_print();
		return 0;
		}
		else{
			K = atoi(argv[1]);
		 	m1 = atoi(argv[2]);
		 	n = atoi(argv[3]);
		 	m2 = atoi(argv[4]);
		 	btb_size = atoi(argv[5]);
		 	btb_assoc = atoi(argv[6]);
	 		ifstream infile(argv[7]);
	 		filename = argv[7];
		//cout << filename << endl;
			CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		}
	}*/
	string a1;
	a1 = argv[1];
	/*cout << "Going on" << endl;
	if((argv[5] != NULL) && (argv[6] == NULL)){	 
		e1 = argv[5];
		flag1 = 1;
	}
	cout << flag1 << endl;
	if(argv[5] != NULL){                         // Million Dollar Bug
		if(argv[6] != NULL){                         
			e11 = argv[5];
			f1 = argv[6];
			g1 = argv[7];
			flag2 = 1;
		}
	//	cout << "Going on" << endl;
	}
	cout << flag2 << endl;*/
	int special=1;
	if(a1.compare(s1) == 0){
		m2 = atoi(argv[2]);
		//cout << m2 ;
	 	btb_size = atoi(argv[3]);
	 	btb_assoc = atoi(argv[4]);
	 	ifstream infile(argv[5]);
		filename = argv[5];
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		while(infile >> dumb >> dummy){
			a = dummy;	
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			binst = binst.substr((22-m2),m2);
	        	row_info = strtoull(binst.c_str(),NULL,2);
			cachetrial.bimodal(a,row_info);
			i++;
		}
		cachetrial.bimodal_print();
	}
	else if(a1.compare(s2) == 0){
		m1 = atoi(argv[2]);
		 n = atoi(argv[3]);
		 btb_size = atoi(argv[4]);
		 btb_assoc = atoi(argv[5]);
	 	ifstream infile(argv[6]);
	 	filename = argv[6];
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	
		while(infile >> dumb >> dummy){
			a = dummy;	
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			binst = binst.substr((22-m1),m1);
	        	row_info = strtoull(binst.c_str(),NULL,2);
			cachetrial.gshare(a,binst,n);
			//cachetrial.bimodal(a,row_info);
			//break;
			i++;
		}
		cachetrial.gshare_print();
	}
	else if(a1.compare(s3) == 0){
		//cout << "DANG DANG" << endl;
		int brow_info;
		int grow_info;
		 K = atoi(argv[2]);
		 m1 = atoi(argv[3]);
		 n = atoi(argv[4]);
		 m2 = atoi(argv[5]);
		 btb_size = atoi(argv[6]);
		 btb_assoc = atoi(argv[7]);
	 	ifstream infile(argv[8]);
	 	filename = argv[8];
		//cout << filename << endl;
		CACHE cachetrial (K,m1,m2,n,0,0,0,0,0);	 //Start checking from here its function
		while(infile >> dumb >> dummy){
			a = dummy;	
			//cout << special << ". PC: " << dumb << " " << dummy << endl;
			value = strtol(dumb.c_str(), 0, 16);
			binst = DecToBin2(value);
			address_len = 24-binst.length();		
			while(address_len){
				binst.insert(0,parse);
				address_len--;
			}
			//Change from here 
			orig = binst;
			orig2 = binst;
			//cout << binst << endl;
			//cout << m1 << endl;
			orig2 = orig2.substr((22-m1),m1);	
			//cout << orig2 << endl;
			grow_info = cachetrial.hgshare(a,orig2,n); // FAULT HERE
			//cout << grow_info << endl;
			orig = orig.substr((22-m2),m2);
	        	brow_info = strtoull(orig.c_str(),NULL,2);
			//cout << brow_info << " " << K << endl;			
			cachetrial.hbimodal(a,brow_info);
			//cout << K << endl;
			//cout << binst << endl;			
			binst = binst.substr((22-K),K);
			//cout << binst << endl;
	        	row_info = strtoull(binst.c_str(),NULL,2);
			//cout << row_info << endl;			
			cachetrial.hybrid(a,grow_info,brow_info,row_info);
			//cout << row_info << endl;
			//cachetrial.bimodal(a,row_info);
			//cout << "Bang Bang" << endl;
			//break;
			special++;
		}
		cachetrial.hybrid_print();
		cout << "FINAL GSHARE CONTENTS" << endl;
		cachetrial.gshare_print();
		cout << "FINAL BIMODAL CONTENTS" << endl;
		cachetrial.bimodal_print();
	}
	//cout << i << endl;
	/*address_len = 32;	
	double set_bit = totalsize/(blocksize*assoc);
	sets = log(set_bit)/log(2);
	double blksize = log(blocksize)/log(2);
	tag = address_len - sets - blksize; // MAIN processing starts from Next line have to iterate over it
	int passblksize = blksize; //Block Offset
	int setnum = sets; //Set Bits
	int tagnum = tag; //Tag Size
	int set_row = set_bit;
	CACHE cachetrial (tagnum,set_row,setnum,passblksize,assoc);	
	for(int k=0;k<i;k++)
	{
		if((a[k] == 'r') || (a[k] == 'R')){
			cachetrial.readFromAddress(binst[k],tagnum,setnum,passblksize,repolicy,assoc);
		}
		else if((a[k] == 'w') || (a[k] == 'W')){
			cachetrial.writeToAddress(binst[k],tagnum,setnum,passblksize,writepolicy,repolicy,assoc);
		}
	}
	startprint(blocksize,totalsize,assoc,repolicy,writepolicy,filename);
	cachetrial.checkprint(set_row,assoc,tag,writepolicy,totalsize,blocksize);*/
	return 0;
}
