#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
int main(int argc, char **argv)
{
  if(argc >= 3){
    std::string command = argv[1];
    std::string keylog_location(argv[argc-1]);
    int parent_to_child[2];
    int child_to_parent[2];
    int child_to_parent_stderr[2];
    if(pipe(parent_to_child) != 0){
      std::cerr << "Error in creating pipe" << std::endl;
      return 1;
    }
    if(pipe(child_to_parent) != 0){
      std::cerr << "Error in creating pipe" << std::endl;
      return 1;
    }
    if(pipe(child_to_parent_stderr) != 0){
      std::cerr << "Error in creating pipe" << std::endl;
      return 1;
    }
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
      argv[argc - 1] = NULL;
      execv(exec_name.c_str(), &argv[1]);
    }else if (pid > 0) {
      // parent process
      int j = 0;
      for (; j < 5; ++j)
        {
	  printf("parent process: counter=%d\n", ++counter);
        }
    }
  else
    {
      // fork failed
      printf("fork() failed!\n");
      return 1;
    }

  printf("--end of program--\n");

  return 0;
}
