#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
struct entry{
  //delay in milliseconds
  unsigned long long delay;
  std::string line;
};


void line_listen(const std::vector<entry>& entries, std::mutex lock, int write_pipe){
  auto start = std::chrono::high_resolution_clock::now();
  for (std::string line; std::getline(std::cin, line);){
    lock.lock();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_object = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    unsigned long long delay = (unsigned long long) elapsed_object.count();
    entry entry;
    entry.delay = delay;
    entry.line.assign(line);
    entries.push_back(entry);
    lock.unlock();
    std::string with_newline = line.append("\n");
    write(write_pipe, with_newline.c_str(), sizeof(char)*strlen(with_newline.c_str()));
    start = end;
  }
}

int main(int argc, char **argv)
{
  if(argc >= 3){
    std::string command = argv[1];
    std::string keylog_location(argv[argc-1]);
    int parent_to_child[2];
    int child_to_parent[2];
    int child_to_parent_stderr[2];
    if(pipe(parent_to_child) != 0){
      std::cerr << "error in creating pipe" << std::endl;
      return 1;
    }
    if(pipe(child_to_parent) != 0){
      std::cerr << "error in creating pipe" << std::endl;
      return 1;
    }
    if(pipe(child_to_parent_stderr) != 0){
      std::cerr << "error in creating pipe" << std::endl;
      return 1;
    }
    pid_t pid = fork();
    if (pid == 0){
      //this is the child process
      //redirect parent_to_child into standard input, standard output to child_to_parent, and standard errer to child_to_parent_error
      //fd[0] is read end, fd[1] is the write end
      /*
	close write end of parent_to_child, and read ends of child_to_parent and child_to_parent_error
       */
      close(parent_to_child[1]);
      close(child_to_parent[0]);
      close(child_to_parent_stderr[0]);
      dup2(0, parent_to_child[0]);
      close(0);
      dup2(1, child_to_parent[1]);
      close(1);
      dup2(2, child_to_parent_stderr[1]);
      close(parent_to_child[0]);
      close(child_to_parent[1]);
      close(child_to_parent_stderr[1]);
      std::size_t whitespace_location = command.find(" ");
      std::string exec_name(command, 0, whitespace_location);
      argv[argc - 1] = null;
      execv(exec_name.c_str(), &argv[1]);
    }else if (pid > 0) {
      close(parent_to_child[0]);
      close(child_to_parent[1]);
      close(child_to_parent_stderr[1]);
      dup2(1, child_to_parent[0]);
      close(child_to_parent[0]);
      dup2(2, child_to_parent_stderr[0]);
      close(child_to_parent_stderr[0]);
      int status;
      std::vector<entry> entries;
      std::mutex lock;
      std::thread reading_thread(line_listen, entries, lock, parent_to_child[1]);
      pid_t result = waitpid(pid, &status);
      if(result == -1){
	std::cout << "error with running binary. terminating." << std::endl;
      }else{
	//write the entries.
	std::ofstream file;
	file.open(keylog_location);
	for(std::vector<entry>::iterator iter = entries.begin(); iter != entries.end(); ++iter){
	  unsigned long long delay = iter->delay;
	  string line = iter->line;
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

  return 0;
}
