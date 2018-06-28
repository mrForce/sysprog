#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
struct Entry{
  //delay in milliseconds
  unsigned long long delay;
  std::string line;
  Entry(unsigned long long d, std::string& l) : delay(d), line(std::move(l)){}
};

std::vector<Entry> read_entry_file(std::string filename){
  std::ifstream file;
  file.open(filename);
  std::string line;
  std::vector<Entry> entries;
  while(std::getline(file, line)){
    std::size_t comma_location = line.find(",");
    if(comma_location == std::string::npos){
      std::cout << "ERROR! No comma in entry file line" << std::endl;
      exit(1);
    }
    std::string delay_string = line.substr(0, comma_location);
    unsigned long long num_milliseconds = std::stoull(delay_string);
    std::string typed_line = line.substr(comma_location + 1);
    std::cout << "Typed line: " << typed_line << std::endl;
    entries.emplace_back(num_milliseconds, typed_line);
  }
  return entries;
}

int main(int argc, char **argv)
{
  /* 
     Four args: entry file, multiplier, binary, output file
  */
  if(argc == 5){
    std::string entry_file_name(argv[1]);
    std::string multiplier_string(argv[2]);
    std::string binary_file_name(argv[3]);
    std::string output_file_name(argv[4]);
    std::vector<Entry> entries = read_entry_file(entry_file_name);
  }else{
    std::cout << "Wrong number of arguments" << std::endl;
    return 1;
  }

  return 0;
}
