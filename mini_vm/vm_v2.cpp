#!/bin/bash
set -e; umask 077;
COMP='/usr/bin/gcc'
FLAGS='-std=c++11 -Wall -Wextra -O0 -g3 -DDEBUG -x c++'
LINK='-lstdc++ -fsanitize=address'
LINE_EOF=$(( $(grep -n "^#!EOF\$" vm.cpp | grep -o "^[0-9]*") +1))
if [ "$0" -nt "$0.exe" ]; then
  sed -n -e ''"$LINE_EOF"',$p' "$0" | $COMP $FLAGS -o "$0.exe" - $LINK
fi
set +e; timeout 1 $0.exe "$0" "$@"; STATUS=$?;
exit $STATUS
#!EOF
#line 14 "vm.cpp"
//hexdump -v -C
// logger macros
#include "macro_log.h"

// c includes
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <cassert>

// c++ includes
#include <vector>

// helpers
typedef uint64_t ddword;
typedef uint32_t  dword;
typedef uint16_t   word;
typedef uint8_t    byte;

template<typename TYPE>
byte extract_byte(TYPE data, byte byte) {
  return 0xFF & (data >> (CHAR_BIT * byte));
}

template<typename TYPE>
bool extract_bit(TYPE data, byte bit) {
  return (0x01 & (data >> bit)) ? true : false;
}

// memory organisation
template <typename T> 
struct Register {
  T type;
  T value;
};

template <typename T> 
struct Bank {
  Bank(T start, T length):
      start{start},end{start+length},length{length} {
    data = data[length];
  }
  T data[];
  const T start;
  const T end;
  const T length;
};

template <typename T> struct VM;
template <typename T>
using InstPtr = void(*)(VM<T>*, const T);
/*typedef void (*InstPtr)(VM<T>* ctx, const T);*/

template <typename T> 
struct VM {
  Register<T>          regFile[16];
  InstPtr<T>           instTable[256];   
  std::vector<Bank<T>> memBank;
  word                 state;
  const byte           rom[];
  byte*                pc = nullptr;
  bool                 isRunning = true;
  
  VM(const byte program_bin[]).rom(program_bin), pc(program_bin){
    clear_regFile();
    clear_instTable();
  }
  void clear_regFile(){
    size_t regFile_len = sizeof(regFile)/sizeof(regFile[0]);
    for( size_t i=regFile_len; i>0; --i ){
      regFile[i-1u].type = 0;
      regFile[i-1u].value = 0;
    }
  }
  
  void clear_instTable(){
    size_t instTable_len = sizeof(instTable)/sizeof(instTable[0]);
    for( size_t i=instTable_len; i>0; --i ){
      instTable[i-1u] = nullptr;
    }
  }
  
  void dispatch(const T instr){
    const byte i = extract_byte<T>(instr,0);
    if(instTable[i] != nullptr)
      instTable[i](this, instr);
    else
      err("Non valid instruction");
  }
  
  void step(){
    // Read a 32-bit bytecode instruction.
    byte instr_header = *pc;
    byte instr_len    = instr_header & 0x0f;
    byte instr_con    = instr_header & 0xf0;
    T instr           = 0;
    assert( instr_len <= sizeof(T) );
    // fetch
    for(size_t i=1; i<=instr_len; ++i){
      instr += (*pc)[i] << (CHAR_BIT * i);
    }
    
    if(instr_con != 0 && extract_bit<word>(state,instr_con)){
      // Dispatch to an opcode-handler.
      dispatch(instr);
    }
    // Increment to next instruction.
    pc += instr_len;
  }
  
  void run(){
    while(isRunning){
      step();
    }
  }
};

// instruction defs
template <typename T> 
void nop(VM<T>*, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  const byte b = extract_byte<T>(instr,2);
  const byte c = extract_byte<T>(instr,3);
}

bool is_running = true;
template <typename T> 
void halt(VM<T>*, const T) {
  is_running = false;
}


int main(void) {
  VM<dword> vm;
  // Initialize funptr table:
  vm.instTable[0x23] = halt;           // '#' in the bytecode.
 
  
  // Main loop:
  for(dword i=0; is_running; ++i){ ; }
  
  log("EXIT SUCCESS");
  exit(EXIT_SUCCESS);
}
