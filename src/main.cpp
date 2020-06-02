#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>

typedef struct block {
  int lru;
  int tag;
  int valid;
} block; 
int mode ;
// Some global variable
std::string log_file_type = ".log";
auto start = std::chrono::system_clock::now(); // Start time
auto end = std::chrono::system_clock::now();   // End time
std::chrono::duration<double> elapsed_seconds =
    end - start; // Do some calculation of time
std::time_t end_time = std::chrono::system_clock::to_time_t(end);

int size1[16384];
int lru1[16384];

int size2[16384];
int lru2[16384];

block inst_cache[16384][2];
block data_cache[16384][4];
int call[10000];
int cache[10000];
int n = 0;
int readhit, readmiss, writehit, writemiss, read, write;
void Inputfile();
int countindex(int );
int counttag(int );
void readindata(int , int , int ); 
void writeindata(int , int , int );
void readininstr(int , int , int );
void evict(int , int , int);
void erase_cache();
void INFO();
void GUI();
void loop();
int main(int argc, char *argv[]) {
    std::cout<<" Type in mode: ";std::cin>>mode;
  	GUI();
  	Inputfile();
  	loop();

  return 0;
}
void Inputfile() {
  std::ifstream file("data.txt");
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
int countindex(int data) {
  int offsets;
  data = data / 64;
  offsets = data % 16384;
  return offsets;
}
int counttag(int data) {
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
  output.open(std::ctime(&end_time) + log_file_type,
               std::ios::out | std::ios::app);
  output << "========================" << std::endl;
  output << "# cache reads :" << read << std::endl;
  output << "# cache writes :" << write << std::endl;
  output << "========================" << std::endl;
  output << "Read miss : " << readmiss << std::endl;
  output << "Write miss :" << writemiss << std::endl;
  output << "========================" << std::endl;
  output << "Read hit : " << readhit << std::endl;
  output << "Write hit :" << writehit << std::endl;
  output << "========================" << std::endl;
  output << "# cache hits: " << readhit + writehit << std::endl;
  output << "# cache misses :" << readmiss + writemiss << std::endl;
  double  rate = (double)(readhit + writehit) / (read + write) * 100;
  output << "Cache hit ratio :" << std::fixed << std::setprecision(2) << rate << "%" << std::endl;
  output << "========================" << std::endl;
  output.close();
}

void erase_cache() {
	block block;


  std::fstream output;
  output.open(std::ctime(&end_time) + log_file_type,
               std::ios::out | std::ios::app);
  output << "XX -CACHE CLEAR - XX" << std::endl;
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
  block = {};
  output.close();
}
void GUI() {
  std::fstream output;
  output.open(std::ctime(&end_time) + log_file_type, std::ios::out);
  output << "========================" << std::endl;
  output.close();
}
void loop(){
for (int i = 1; i <= n; i++) {
    int index, tag;
    index = countindex(cache[i]);
    tag = counttag(cache[i]);
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
      output << "--------------------------" << std::endl;
      output.close();
      INFO();
	  break;
    }
    std::cout << i << std::endl;
  }
}