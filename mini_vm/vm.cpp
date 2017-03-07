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
#ifndef NDEBUG
  #ifdef DEBUG
    #define __FUNC__ __PRETTY_FUNCTION__
    #define POS_STR "%s(%d)[%s]\n  "
    #define POS_MAC __FILE__,__LINE__,__FUNC__,
  #else
    #define POS_STR
    #define POS_MAC
  #endif
  
  #define log(msg) do{fprintf(stderr,POS_STR "Log:  \e[34m %s\e[0m\n",POS_MAC (msg));}while(0);
  #define wrn(msg) do{fprintf(stderr,POS_STR "Warn: \e[33m %s\e[0m\n",POS_MAC (msg));}while(0);
  #define err(msg) do{fprintf(stderr,POS_STR "Error:\e[31m %s\e[0m\n",POS_MAC (msg));}while(0);
#else
  #define log(msg) do{(void)0;}while(0);
  #define wrn(msg) do{(void)0;}while(0);
  #define err(msg) do{(void)0;}while(0);
#endif

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

template <typename T> 
struct VM;

template <typename T>
using InstPtr = void(*)(VM<T>*, const T);
/*template <typename T> 
typedef void (*InstPtr)(VM<T>* ctx, const T);*/

template <typename T> 
struct VM {
  Register<T>          regFile[16];
  InstPtr<T>           instTable[256];   
  std::vector<Bank<T>> memBank;
  
  VM(){
    size_t regFile_len = sizeof(regFile)/sizeof(regFile[0]);
    for( size_t i=regFile_len; i>0; --i ){
      regFile[i-1u].type = 0;
      regFile[i-1u].value = 0;
    }
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
  
  void step(T** pc){
    // Read a 32-bit bytecode instruction.
    T instr = **pc;
    // Dispatch to an opcode-handler.
    dispatch(instr);
    // Increment to next instruction.
    (*pc)++;
  }
};

// instruction defs
template <typename T> 
void add(VM<T>* ctx, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  const byte b = extract_byte<T>(instr,2);
  const byte c = extract_byte<T>(instr,3);
  ctx->regFile[a].value = 
      ctx->regFile[b].value + ctx->regFile[c].value;
}

template <typename T> 
void sub(VM<T>* ctx, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  const byte b = extract_byte<T>(instr,2);
  const byte c = extract_byte<T>(instr,3);
  ctx->regFile[a].value = 
      ctx->regFile[b].value - ctx->regFile[c].value;
}

template <typename T> 
void multiply(VM<T>* ctx, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  const byte b = extract_byte<T>(instr,2);
  const byte c = extract_byte<T>(instr,3);
  ctx->regFile[a].value = 
      ctx->regFile[b].value * ctx->regFile[c].value;
}

template <typename T> 
void divide(VM<T>* ctx, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  const byte b = extract_byte<T>(instr,2);
  const byte c = extract_byte<T>(instr,3);
  ctx->regFile[a].value = 
      ctx->regFile[b].value / ctx->regFile[c].value;
}

template <typename T> 
void mod(VM<T>* ctx, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  const byte b = extract_byte<T>(instr,2);
  const byte c = extract_byte<T>(instr,3);
  ctx->regFile[a].value = 
      ctx->regFile[b].value % ctx->regFile[c].value;
}

template <typename T> 
void load_immediate(VM<T>* ctx, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  const byte b = extract_byte<T>(instr,2);
  //const byte c = extract_byte<word>(instr,3); //Not used
  ctx->regFile[a].value = b;
}

bool is_running = true;
template <typename T> 
void halt(VM<T>*, const T) {
  is_running = false;
}

template <typename T> 
void print(VM<T>* ctx, const T instr) {
  const byte a = extract_byte<T>(instr,1);
  printf("reg[%2u] = '%c' = %u:%x   \n", a,
      ctx->regFile[a].value,
      ctx->regFile[a].value, ctx->regFile[a].type
      );
}

int main(void) {
  VM<dword> vm;
  // Initialize funptr table:
  vm.instTable[0x2B] = add;            // '+' in the bytecode.
  vm.instTable[0x2D] = sub;            // '-' in the bytecode.
  vm.instTable[0x2A] = multiply;       // '*' in the bytecode.
  vm.instTable[0x2F] = divide;         // '/' in the bytecode.
  vm.instTable[0x25] = mod;            // '%' in the bytecode.
  vm.instTable[0x24] = load_immediate; // '$' in the bytecode.
  vm.instTable[0x3F] = print;          // '?' in the bytecode.
  vm.instTable[0x23] = halt;           // '#' in the bytecode.
  
  dword instBuffer[] = {
    0x00410224,
    0x0000023f,
    0x00000023,
  };
  dword* pc = instBuffer;
  
  // Main loop:
  for(dword i=0; is_running; ++i){ vm.step( &pc ); }
  assert(vm.regFile[2].value == 65);
  is_running = true;

  dword instBuffer2[] = {
    0x00010024,
    0x00020124,
    0x00030224,
    0x0102012b,
    0x0001002b,
    0x0000003f,
    0x00000023,
  };
  pc = instBuffer2;
  
  // Main loop:
  for(dword i=0; is_running; ++i){ vm.step( &pc ); }
  assert(vm.regFile[0].value == 6);
  
  log("EXIT SUCCESS");
  exit(EXIT_SUCCESS);
}
