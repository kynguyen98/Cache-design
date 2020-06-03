#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <cstdlib>
/* in a block of data that has 16k set 
  * each set has 4 line corresponding 4 way data
  * block of instruction still has 16k set but each set has 2 lines or 2 way
*/
typedef struct {
  int lru;
  int tag;
  int valid;
}block ; 

int mode ;
void Inputfile(std::string);
int find_index(int );
int find_tag(int );
void readindata(int , int , int ); 
void writeindata(int , int , int );
void readininstr(int , int , int );
void evict(int , int , int);
void erase_cache();
void INFO();
void loop();
// Some global variable
std::string log_file_type = ".log";
auto start = std::chrono::system_clock::now(); // Start time
auto end = std::chrono::system_clock::now();   // End time
std::chrono::duration<double> elapsed_seconds =
    end - start; // Do some calculation of time
std::time_t end_time = std::chrono::system_clock::to_time_t(end);

// Because the ache is 16k set so we consider the size1 which is the size of the cache 
int size1[16384];
int lru1[16384]; // lru for the cache when we use 2 way set associated to update the last bit been used 

int size2[16384]; // size2 would be the size of the memory outside the cache we want to take away from 
int lru2[16384];

block inst_cache[16384][2];
/* 16384 just to store the index or to find the index and [2] for valid and dirt
* like 1 block 1 set and in 1 set has 2 way instruction and 4 way for data cache
* sau khi nhap dia chi tu file vo thi tinh tag + index roi dung index chi vo cai way instruction cache de cho 2 cai cung index
* cung index roi so sanh cai tag
*/
block data_cache[16384][4];
int call[10000];
int cache[10000];
int n = 0;
/*
* tag giong nhau thi read hit ko thi read miss
* write thi phai kiem tra no day chua 
*/
int readhit, readmiss, writehit, writemiss, read, write;


int main(int argc, char *argv[]) {

    if (argc < 2){
      // Tell the user how to run the program
      std::cout<<"ERROR! Incorrect argument!"<<std::endl;
      std::cerr << "Usage: " << "./main [DATANAME] [MODE]" << std::endl;
      		// how to run a program when they enter command incorrectly
      return 1;
    }
    std::string filename(argv[1]);
    mode =  atoi(argv[2]);
  	Inputfile(filename);
  	loop();
    std::cout<<"Please check the log file!"<<std::endl;
}
void Inputfile(std::string filename) {
  std::ifstream file(filename);
  if (file.fail()) {
    std::cout << "Failed to open this file!" << std::endl;
  } else {
    while (!file.eof()) {
      n++;
      file >> std::dec >> call[n] >> std::hex >> cache[n];
    }
  }
  file.close();
}
int find_index(int data) {
  int offsets;
  data = data / 64;
  offsets = data % 16384;
  return offsets;
}
int find_tag(int data) {
  int tag;
  data = data / 64;
  tag = data / 16384;
  return tag;
}
void readindata(int data, int tag, int index) {
  std::fstream file;
  file.open(std::ctime(&end_time) + log_file_type,
               std::ios::out | std::ios::app);
  int tick = 0;
  lru1[index]++;
  for (int i = 1; i <= 4; i++) {
    if (tag == data_cache[index][i].tag) {
      readhit++;
      tick = 1;
      data_cache[index][i].lru = lru1[index];
    }
  }
  if (tick == 0) {
    readmiss++;
	if (mode == 2 ){
     file << "Read from L2 0x" << std::hex << data << std::endl;
	}

    if (size1[index] < 4) {
      size1[index] = size1[index] + 1;
      data_cache[index][size1[index]].tag = tag;
      data_cache[index][size1[index]].lru = lru1[index];
    } else {
      int l, vt;
      l = data_cache[index][1].lru;
      vt = 1;
      for (int i = 2; i <= 4; i++) {
        if (l > data_cache[index][i].lru) {
          l = data_cache[index][i].lru;
          vt = i;
        }
      }

      data_cache[index][vt].tag = tag;
      data_cache[index][vt].lru = lru1[index];
    }
  }
  file.close();
}

void writeindata(int data, int tag, int index) {
  std::fstream file;
  file.open(std::ctime(&end_time) + log_file_type,
               std::ios::out | std::ios::app);
  int tick = 0;
  lru1[index]++;
  for (int i = 1; i <= 4; i++) {
    if (tag == data_cache[index][i].tag) {
      writehit++;
      tick = 1;
      data_cache[index][i].lru = lru1[index];
    }
  }
  if (tick == 0) {
    writemiss++;
	if (mode == 2 ){
    file << "Read for Ownership from L2 0x" << std::hex << data << std::endl;
	}

    if (size1[index] < 4) {
      size1[index] = size1[index] + 1;
      data_cache[index][size1[index]].tag = tag;
      data_cache[index][size1[index]].lru = lru1[index];
    } else {
      int l, vt;
      l = data_cache[index][1].lru;
      vt = 1;
      for (int i = 2; i <= 4; i++) {
        if (l > data_cache[index][i].lru) {
          l = data_cache[index][i].lru;
          vt = i;
        }
      }

      data_cache[index][vt].tag = tag;
      data_cache[index][vt].lru = lru1[index];
    }
  }
  file.close();
}


void readininstr(int data, int tag, int index) {
  std::fstream file;
  file.open(std::ctime(&end_time) + log_file_type,
               std::ios::out | std::ios::app);
  int tick = 0;
  lru2[index]++;
  for (int i = 1; i <= 2; i++) {
    if (tag == inst_cache[index][i].tag) {
      readhit++;
      tick = 1;
      inst_cache[index][i].lru = lru2[index];
    }
  }
  if (tick == 0) {
    readmiss++;
	if (mode == 2){
   		file << "Read from L2 0x" << std::hex << data << std::endl;
	}
 
    if (size2[index] < 2) {
      size2[index] = size2[index] + 1;
      inst_cache[index][size2[index]].tag = tag;
      inst_cache[index][size2[index]].lru = lru2[index];
    } else {
      int l, vt;
      l = inst_cache[index][1].lru;
      vt = 1;
      if (l > inst_cache[index][2].lru) {
        l = inst_cache[index][2].lru;
        vt = 2;
      }
      inst_cache[index][vt].tag = tag;
      inst_cache[index][vt].lru = lru2[index];
    }
  }
  file.close();
}

void evict(int data, int tag, int index) {
  std::cout << "OFF " << data << std::endl;
  if (data < 16777216) {
    for (int i = 1; i <= 2; i++) {
      if (inst_cache[index][i].tag == tag) {
        inst_cache[index][i].tag = 0;
        inst_cache[index][i].lru = 0;
      }
    }
  } else {
    for (int i = 1; i <= 4; i++) {
      if (data_cache[index][i].tag == tag) {
        data_cache[index][i].tag = 0;
        data_cache[index][i].lru = 0;
      }
    }
  }
}
void INFO() {
  std::fstream output;
  output.open(std::ctime(&end_time) + log_file_type, std::ios::out | std::ios::app);
  output << "[LOG] Mode: " << mode << std::endl;
  output << "========================" << std::endl;
  output << "> #reads         :" << read << std::endl;
  output << "> #writes        :" << write << std::endl;
  output << "> Read miss      :" << readmiss << std::endl;
  output << "> Write miss     :" << writemiss << std::endl;
  output << "> Read hit       :"  << readhit << std::endl;
  output << "> Write hit      :" << writehit << std::endl;
  output << "> #cache hits    :" << readhit + writehit << std::endl;
  output << "> #cache misses  :" << readmiss + writemiss << std::endl;
  double  rate = (double)(readhit + writehit) / (read + write) * 100;
  output << "> hit ratio      :" << std::fixed << std::setprecision(2) << rate << "%" << std::endl;
  output << "========================" << std::endl;
  output << std::endl;
  output.close();
}

void erase_cache() {
  std::fstream output;
  block block;
  output.open(std::ctime(&end_time) + log_file_type, std::ios::out | std::ios::app);
  if (mode == 2){
  output << "XX -CACHE CLEAR - XX" << std::endl;
  }
  std::fill(std::begin(size1), std::end(size1), 0);
  std::fill(std::begin(lru1), std::end(lru1), 0);
  std::fill(std::begin(size2), std::end(size2), 0);
  std::fill(std::begin(lru2), std::end(lru2), 0);
  readhit = 0;
  readmiss = 0;
  writehit = 0;
  writemiss = 0;
  read = 0;
  write = 0;
  std::memset(&block, 0, sizeof(block));
  output.close();
}
void loop(){
for (int i = 1; i <= n; i++) {
    int index, tag;
    index = find_index(cache[i]);
    tag = find_tag(cache[i]);
	switch (call[i]){
    case 0: 
      read++;
      readindata(cache[i], tag, index);
	break;
    case 1: 
      write++;
      writeindata(cache[i], tag, index);
    break;
    case 2:
      read++;
      readininstr(cache[i], tag, index);
    break;
    case 3:
      evict(cache[i], tag, index);
    break;
    case 8:
      erase_cache();
    break;
    case 9:
      std::fstream output;
      output.open(std::ctime(&end_time) + log_file_type,
                   std::ios::out | std::ios::app);
      output << "========================" << std::endl;
      output.close();
      INFO();
	  break;
    }
  }
}