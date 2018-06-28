#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
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
     Four args: entry file, multiplier, command,  output file
  */
  if(argc >= 5){
    std::string entry_file_name(argv[1]);
    std::string multiplier_string(argv[2]);
    int multiplier = std::stoi(multiplier_string);
    std::string exec_name(argv[3]);
    std::string output_file_name(argv[argc-1]);
    std::vector<Entry> entries = read_entry_file(entry_file_name);
    int parent_to_child[2];
    int child_to_parent[2];
    if(pipe(parent_to_child) != 0){
      std::cerr << "error in creating pipe" << std::endl;
      return 1;
    }
    if(pipe(child_to_parent) != 0){
      std::cerr << "Error in creating pipe" << std::endl;
      return 1;
    }
    pid_t pid = fork();
    if (pid == 0){
      //this is the child process
      //fd[0] is read end, fd[1] is the write end
      close(parent_to_child[1]);
      close(child_to_parent[0]);
      dup2(parent_to_child[0], 0);
      close(parent_to_child[0]);
      dup2(child_to_parent[1], 1);
      close(child_to_parent[1]);
      std::size_t whitespace_location = command.find(" ");
      std::string exec_name(command, 0, whitespace_location);
      argv[argc - 1] = NULL;
      std::cout << "Going to call execv";
      execv(exec_name.c_str(), &argv[3]);
    }else if (pid > 0) {
      std::vector<Entry> entries = read_entry_file(entry_file_name);
      close(parent_to_child[0]);
      close(child_to_parent[1]);
      for(std::vector<Entry>::iterator iter = entries.begin(); iter != entries.end(); ++iter){
	unsigned long long delay = iter->delay;
	std::string line = iter->line;
	fd_set rfds;
	struct timeval tv;
	int retval;
      }
    }
  }else{
    std::cout << "Wrong number of arguments" << std::endl;
    return 1;
  }

  return 0;
}
