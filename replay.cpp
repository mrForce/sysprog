#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sstream>
#include <sys/wait.h> 
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <fcntl.h>
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


void read_for_time(unsigned long long total_delay, std::ofstream& output_file, int read_pipe ){
  auto start_time = std::chrono::high_resolution_clock::now();
  
  std::stringstream ss;
  char* characters = (char*) malloc(100*sizeof(char));
  std::vector<char> char_vec;
  while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() <= total_delay){
    ssize_t num_bytes_read = read(read_pipe, characters, 99);

    if(num_bytes_read > 0){
      std::cout << "num bytes read: " << num_bytes_read << std::endl;
      for(int i = 0; i < num_bytes_read; i++){       
	  char_vec.push_back(characters[i]);
      }
    }
  }
  free(characters);
  std::cout << "Time spent reading: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << std::endl;
  for(std::vector<char>::iterator iter = char_vec.begin(); iter != char_vec.end(); ++iter){
    output_file << *iter;
  }

}


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
    std::cout << "Typed line: " << typed_line << std::endl << std::flush;
    entries.emplace_back(num_milliseconds, typed_line);
  }
  return entries;
}

int main(int argc, char **argv)
{
  /* 
     Five args: entry file, multiplier, time before stopping, command,  output file
  */
  if(argc < 6){
    std::cout << "Usage: ./replay entry_file_name multiplier end_wait_time exec_name (exec args) output_file_name" << std::endl;
  }else{
    std::string entry_file_name(argv[1]);
    std::string multiplier_string(argv[2]);
    std::string end_wait_string(argv[3]);
    int multiplier = std::stoi(multiplier_string);
    int end_wait_time = std::stoi(end_wait_string);
    std::string exec_name(argv[4]);
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
    
    /* Read non-blocking from child.*/
    if(fcntl(child_to_parent[0], F_SETFL, O_NONBLOCK) < 0){
      exit(2);
    }
    
    pid_t pid = fork();
    
    if (pid == 0){
      //this is the child process
      //fd[0] is read end, fd[1] is the write end
      //std::cout << "Exec name: " << exec_name << std::endl;
      close(parent_to_child[1]);
      close(child_to_parent[0]);
      dup2(parent_to_child[0], 0);
      close(parent_to_child[0]);
      dup2(child_to_parent[1], 1);
      close(child_to_parent[1]);
      
      argv[argc - 1] = NULL;
      //std::cout << "Going to call execv" << std::endl;
      /*
        std::string line_one, line_two, line_three;
	std::getline(std::cin, line_one);
	//std::cout << "Got first line" << std::endl;
  std::getline(std::cin, line_two);
  //std::cout << "Second line" << std::endl;
  std::getline(std::cin, line_three);
  //  std::cout << "Third line" << std::endl;
  std::cout << line_three << std::endl;
  std::cout << line_two << std::endl;
  std::cout << line_one << std::endl;*/

  execv(exec_name.c_str(), &argv[4]);
  //  std::cerr << "execv failed" << std::endl;
    }else if (pid > 0) {
      //std::vector<Entry> entries = read_entry_file(entry_file_name);
      std::ofstream output_file;
      output_file.open(output_file_name, std::ios::out);
      close(parent_to_child[0]);
      close(child_to_parent[1]);
      int i = 0;
      for(std::vector<Entry>::iterator iter = entries.begin(); iter != entries.end(); ++iter){
	std::cout << "i: " << i << std::endl;
	i++;
	unsigned long long delay = iter->delay;
	std::string line = (iter->line).append("\n");
	std::cout << "Line: " << line;
	
	unsigned long long total_delay = delay*multiplier;
	read_for_time(total_delay, output_file, child_to_parent[0]);
	const char* c_string = line.c_str();
	ssize_t num_bytes_written = 0;
	//num_bytes_written = write(parent_to_child[1], c_string, strlen(c_string) + 1);
	//std::cout << "Num bytes written: " << num_bytes_written << std::endl;
	
	while(num_bytes_written < strlen(c_string)){
	  num_bytes_written += write(parent_to_child[1], &c_string[num_bytes_written], strlen(&c_string[num_bytes_written]));	  
	}
	output_file << line;
      }
      unsigned long long time_ull = (unsigned long long) end_wait_time;
      read_for_time(time_ull*1000, output_file, child_to_parent[0]);
      output_file.close();
    }
  }
  return 0;
}
