#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <sys/types.h>


#include <fstream>


int main(int argc, char **argv)
{
  int num_repeat = 1;
  if(argc == 2){
    num_repeat = atoi(argv[1]);
  }
  std::string line_one, line_two, line_three;
  std::getline(std::cin, line_one);
  std::getline(std::cin, line_two);
  std::getline(std::cin, line_three);
  for(int i = 0; i < num_repeat; i++){    
    std::cout << line_three << std::endl;
    std::cout << line_two << std::endl;
    std::cout << line_one << std::endl;
  }
  while(1){
    std::cout << "hello";
  }
  return 0;
}
