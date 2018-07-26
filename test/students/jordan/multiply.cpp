#include <stdio.h>
#include <iostream>
#include <string>
int main(int argc, char** argv){
  if(argc == 2){
    std::string multiplier_string = argv[1];
    int multiplier = std::stoi(multiplier_string);
    std::string line;
    std::getline(std::cin, line);
    std::cout << multiplier*std::stoi(line) << std::endl;
    std::getline(std::cin, line);
    std::cout << multiplier*std::stoi(line) << std::endl;
    std::getline(std::cin, line);
    std::cout << multiplier*std::stoi(line) << std::endl;
  }else{
    std::cout << "usage: multiply <multiplier>" << std::endl;
  }
}
