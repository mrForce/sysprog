#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
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
};


void line_listen(std::vector<Entry>* entries, std::mutex* lock, int write_pipe, char* finished){
  auto start = std::chrono::high_resolution_clock::now();
  for (std::string line; std::getline(std::cin, line);){
    lock->lock();
    if(*finished){
      break;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_object = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    unsigned long long delay = (unsigned long long) elapsed_object.count();
    Entry entry;
    entry.delay = delay;
    entry.line.assign(line);
    entries->push_back(entry);
    lock->unlock();
    std::string with_newline = line.append("\n");
    write(write_pipe, with_newline.c_str(), sizeof(char)*strlen(with_newline.c_str()));
    start = end;

  }
}

int main(int argc, char **argv)
{
  if(argc < 3){
    std::cout << "Usage: ./capture exec_name [exec args] output_file" << std::endl;
    return 1;
  }
  if(argc >= 3){
    std::string command = argv[1];
    std::string keylog_location(argv[argc-1]);
    int parent_to_child[2];
    if(pipe(parent_to_child) != 0){
      std::cerr << "error in creating pipe" << std::endl;
      return 1;
    }
    pid_t pid = fork();
    if (pid == 0){
      //this is the child process
      //fd[0] is read end, fd[1] is the write end
      close(parent_to_child[1]);
      dup2(parent_to_child[0], 0);
      close(parent_to_child[0]);
      std::size_t whitespace_location = command.find(" ");
      std::string exec_name(command, 0, whitespace_location);
      argv[argc - 1] = NULL;
      std::cout << "Going to call execv";
      execv(exec_name.c_str(), &argv[1]);
      std::cout << "done with execv";
    }else if (pid > 0) {
      close(parent_to_child[0]);
      int status;
      std::vector<Entry> entries;
      std::mutex lock;
      char finished = 0;
      std::thread reading_thread(line_listen, &entries, &lock, parent_to_child[1], &finished);
      std::cout << "Going to wait for waitpid" << std::endl;
      pid_t result = wait(NULL);
      std::cout << "Binary terminated" << std::endl;
      lock.lock();
      finished = 1;
      lock.unlock();
      //wait for it to finish running
      reading_thread.join();
      if(result == -1){
	std::cout << "error with running binary. terminating." << std::endl;
      }else{
	//write the entries.
	std::ofstream file;
	file.open(keylog_location);
	for(std::vector<Entry>::iterator iter = entries.begin(); iter != entries.end(); ++iter){
	  unsigned long long delay = iter->delay;
	  std::string line = iter->line;
	  file << delay << "," << line << std::endl;
	}
	file.close();
	std::cout << "done" << std::endl;
      }
      
    }
  else
    {
      // fork failed
      printf("fork() failed!\n");
      return 1;
    }
  }
  return 0;
}
