#!/bin/bash
set -e; umask 077;
COMP='/usr/bin/gcc'
FLAGS='-std=c++11 -Wall -Wextra -O0 -g3 -x c++'
LINK='-lstdc++'
#-fsanitize=address
LINE_EOF=$(( $(grep -n "^#!EOF\$" "$0" | grep -o "^[0-9]*") +1))
if [ "$0" -nt "$0.exe" ]; then
   sed -n -e ''"$LINE_EOF"',$p' "$0" | $COMP $FLAGS -c -o "$0.out" - 
   $COMP $LINK -o "$0.exe" "$0.out" bstrlib-master/libbstr.a 
fi
set +e; timeout 1 $0.exe "$0" "$@"; STATUS=$?;
exit $STATUS
#!EOF
#line 15 "vm_comp.cpp"
//hexdump -v -C
#include "bstrlib-master/bstrwrap.h"
#include "macro_log.h"
#include <cstdlib>
#include <climits>

#include <string>
#include <iostream> //cout
#include <fstream>  //ifstream 
#include <sstream>
#include <iomanip>  //hex, setfill and set

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// helpers
typedef uint64_t ddword;
typedef uint32_t  dword;
typedef uint16_t   word;
typedef uint8_t    byte;

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

int main(void) {
  //log("TEST LOG");
  //wrn("TEST WARN");
  //err("TEST ERROR");
  
  CBString b;
  std::cout 
      //<< std::showbase      // show the 0x prefix
      << std::internal      // fill between the prefix and the number
      << std::setfill('0'); // fill with 0s
  
  std::ifstream t("vm_base.asm");
  std::string str;
  t.seekg(0, std::ios::end);   
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);
  str.assign((std::istreambuf_iterator<char>(t)),
  std::istreambuf_iterator<char>());
  //std::cout << str << std::endl;
  
  std::istringstream iss(str);
  std::vector<std::string> tokens;
  std::string item;
  char delim = ';';
  while (getline(iss, item, delim)) {
    trim(item);
    if (!item.empty()){
      tokens.push_back(item);
    }
  }
  
  for( std::string token : tokens ){
    std::cout << token << "\t ->";
    std::istringstream iss_token(token);
    std::string element;
    std::vector<std::string> elements;
    char delim = ' ';
    while (getline(iss_token, element, delim)) {
      trim(element);
      if (!element.empty()){
        elements.push_back(element);
      }
    }
    
    byte inst[sizeof(dword)] = {0, 0, 0, 0};
    dword ins = 0;
    for( size_t i=0; i<sizeof(dword) && i<elements.size(); ++i){
      std::string e = elements[i];
      
      try{ inst[i] = std::stoi(e); } 
      catch(std::invalid_argument){ 
        inst[i] = static_cast<byte>(e[0]); }
      ins += inst[i] << (i * CHAR_BIT );
      std::cout << " " << std::hex << std::setw(2) 
          << static_cast<word>(inst[i]);
    }
    std::cout << "\t-> " << std::hex << std::setw(8) 
          << ins << std::endl;
  }
  
  log("EXIT SUCCESS");
  exit(EXIT_SUCCESS);
  return 1;
}
