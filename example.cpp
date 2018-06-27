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
  std::string line_one, line_two, line_three;
  std::getline(std::cin, line_one);
  std::getline(std::cin, line_two);
  std::getline(std::cin, line_three);
  std::cout << line_three << std::endl;
  std::cout << line_two << std::endl;
  std::cout << line_one << std::endl;
  return 0;
}
