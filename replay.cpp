#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pty.h>
#include <utmp.h>

#include <sstream>
#include <sys/wait.h>
#include <poll.h>
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
#include <errno.h>


struct Entry{
  //delay in milliseconds
  unsigned long long delay;
  std::string line;
  Entry(unsigned long long d, std::string& l) : delay(d), line(std::move(l)){}
};

void print_received(std::string str, std::ofstream& out){
  out << "\e[34m" << str <<  "\e[0m" ;
}

void print_sent(std::string str, std::ofstream& out){
  out << "\e[33m" << str << "\e[0m";

}

std::string read_for_time(unsigned long long total_delay, std::ofstream& output_file, int read_pipe ){
  auto start_time = std::chrono::high_resolution_clock::now();
  std::string s;
  int num_characters_read;
  struct pollfd file_descriptors[1];
  file_descriptors[0].fd = read_pipe;
  //set this to listen for read events
  file_descriptors[0].events = POLLIN;
  while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() <= total_delay){
    poll(file_descriptors, 1, total_delay);
    if(file_descriptors[0].revents & POLLIN){
      char* characters = (char*) malloc(100*sizeof(char));
      char keep_going = 1;
      do{
	num_characters_read = read(read_pipe, characters, 99);
	s.append(characters, num_characters_read);
	if(poll(file_descriptors, 1, 0) < 1){
	  keep_going = 0;
	}
      }while(keep_going);
    }else{
      if(file_descriptors[0].revents & POLLERR){
	std::cerr << "POLLERR in reading\n";
      }
      if(file_descriptors[0].revents & POLLHUP){
	std::cerr << "POLLHUP, connection closed\n";
      }

      if(file_descriptors[0].revents & POLLNVAL){
	std::cerr << "POLLNVAL, read_pipe is not open\n";
      }
    }
  }


  
  return s;

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
    //std::cout << "Typed line: " << typed_line << std::endl << std::flush;
    entries.emplace_back(num_milliseconds, typed_line);
  }
  return entries;
}

int main(int argc, char **argv)
{
  /* 
     At least 7 args: entry file, multiplier, time before stopping, command,  output file, valgrind
  */
  if(argc < 7){
    std::cout << "Usage: ./replay entry_file_name multiplier end_wait_time  exec_name (exec args) output_file_name valgrind" << std::endl;
  }else{
    std::string entry_file_name(argv[1]);
    std::string multiplier_string(argv[2]);
    std::string end_wait_string(argv[3]);
    int multiplier = std::stoi(multiplier_string);
    int end_wait_time = std::stoi(end_wait_string);
    std::string exec_name(argv[4]);
    std::string output_file_name(argv[argc-2]);
    std::string valgrind_string(argv[argc-1]);
    int valgrind = std::stoi(valgrind_string);
    //std::vector<Entry> entries = read_entry_file(entry_file_name);

    int parent_to_child[2];
    //int child_to_parent[2];
    int master, slave;
    openpty(&master, &slave, NULL, NULL, NULL);
    if(pipe(parent_to_child) != 0){
      std::cerr << "error in creating pipe" << std::endl;
      return 1;
    }
    /*if(pipe(child_to_parent) != 0){
      std::cerr << "Error in creating pipe" << std::endl;
      return 1;

      }*/
    /*if(fcntl(child_to_parent[0], F_SETFL, O_NONBLOCK) < 0){ 
      exit(2);
      }*/
    //int fw = open("hello.txt", O_APPEND|O_WRONLY);
    pid_t pid = fork();
    

    if (pid == 0){
      login_tty(slave);
      close(master);
      //this is the child process
      //fd[0] is read end, fd[1] is the write end
      //std::cout << "Exec name: " << exec_name << std::endl;
      close(parent_to_child[1]);
      //close(child_to_parent[0]);
      
      dup2(parent_to_child[0], STDIN_FILENO);
      close(parent_to_child[0]);
      /*dup2(child_to_parent[1], STDOUT_FILENO);
      
	close(child_to_parent[1]);*/

      //close(child_to_parent[1]);
      //int b = dup2(fw, 1);
      //close(child_to_parent[1]);
      //close(fw);
      //dup2(fw, 1);
      //close(fw);
      //close(parent_to_child[0]);
      //close(child_to_parent[1]);
      //close(parent_to_child[1]);
      //close(child_to_parent[0]);
      

      if(!valgrind){
	argv[argc - 2] = NULL;
	execv(exec_name.c_str(), &argv[4]);
      }else{
	char* args[argc - 4];
	char vs[] = "valgrind";
	
	args[0] = vs;
	//argv[4] is the executable name
	if(strchr(argv[4], '/')){
	  args[1] = argv[4];
	}else{
	  char* thing = (char*) malloc(sizeof(char)*(3 + strlen(argv[4])));
	  thing[0] = '\0';
	  strcat(thing, "./");
	  strcat(thing, argv[4]);
	  args[1] = thing;
	}
	size_t i = 0;
	for(i = 0; i < argc - 7; i++){
	  args[i + 2] = argv[i + 5];
	}
	args[i + 2] = NULL;
	execvp("valgrind", args);
      }
      std::cerr << "execv failed" << std::endl;
    }else if (pid > 0) {
      /* Read non-blocking from child. That way, if the child process stops writing, we aren't stuck forever. */
     
      /*
      if(close(parent_to_child[0]) == -1){
	std::cerr << "ERROR IN CLOSING PARENT_TO_CHILD[0]\n";
      }
      if(close(child_to_parent[1]) == -1){
	std::cerr << "ERROR IN CLOSING CHILD_TO_PARENT[1]\n";
	}*/
      std::string buf;
      close(slave);
      std::vector<Entry> entries = read_entry_file(entry_file_name);
      std::ofstream output_file;
      std::cout << output_file_name;
      std::cout.flush();
      output_file.open(output_file_name, std::ios::out);
      
      int i = 0;
      for(std::vector<Entry>::iterator iter = entries.begin(); iter != entries.end(); ++iter){

	i++;
	unsigned long long delay = iter->delay;
	std::string line = iter->line;
	line.append("\n");
	//std::cout << "Line: " << line;
	
	unsigned long long total_delay = delay*multiplier;
	
	std::string s = read_for_time(total_delay, output_file, master);
	print_received(s, output_file);
	output_file.flush();
	const char* c_string = line.c_str();
	ssize_t num_bytes_written = 0;
	while(num_bytes_written < strlen(c_string)){
	  //std::cerr << &c_string[num_bytes_written];
	  num_bytes_written += write(parent_to_child[1], &c_string[num_bytes_written], 1);//strlen(&c_string[num_bytes_written]));
	  
	}
	//std::string c = "\n";
	//write(parent_to_child[1], &c, 2);c
	print_sent(line, output_file);
	output_file.flush();
      }

      unsigned long long time_ull = (unsigned long long) end_wait_time;
      std::string s = read_for_time(time_ull*1000, output_file, master);
      print_received(s, output_file);
      output_file.flush();
      output_file.close();
    }
  }
  return 0;
}
