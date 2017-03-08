#!/bin/bash
set -e
umask 077
COMP='/usr/bin/gcc'
FLAGS='-std=c++11 -Wall -Wextra -Werror -O0 -g3 -x c++'
if [ "$0" -nt "$0.out" ]; then
  sed -n -e '14,$p' "$0" | $COMP $FLAGS -o "$0.exe" -
fi
set +e
timeout 1 $0.exe "$0" "$@"
STATUS=$?
exit $STATUS
# bash EOF
#line 15 "mini_cpu.c"
// test: 
//   while true; do timeout 1s ./oneshot_CPU.c ; echo "EXIT:" $?; sleep 2; done
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define log(msg) do{fprintf(stderr,"Log:  \e[34m %s\e[0m\n",(msg));}while(0);
#define wrn(msg) do{fprintf(stderr,"Warn: \e[33m %s\e[0m\n",(msg));}while(0);
#define err(msg) do{fprintf(stderr,"Error:\e[31m %s\e[0m\n",(msg));}while(0);


typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint16   word;
#define WORD_MAX USHRT_MAX

word reg[16u];
word rom[0x100u];
word ram[0x1000u];
word irv[16u]; // inter rupt vector

size_t reg_len  = sizeof(reg)/sizeof(reg[0]);
size_t rom_len  = sizeof(rom)/sizeof(rom[0]);
size_t ram_len  = sizeof(ram)/sizeof(ram[0]);
size_t irv_len  = sizeof(irv)/sizeof(irv[0]);

size_t reg_size = sizeof(reg);
size_t rom_size = sizeof(rom);
size_t ram_size = sizeof(ram);
size_t irv_size = sizeof(irv);

typedef enum{
  REG_PC  = 0xfu,
  REG_RET = 0xeu,
  REG_SP  = 0xdu,
  REG_SF  = 0xcu,
  REG_HP  = 0xbu,
  REG_HF  = 0xau,
} REG_pos_t;

word &pc  = reg[REG_PC];
word &ret = reg[REG_RET];
word &sp  = reg[REG_SP];
word &sf  = reg[REG_SF];
word &hp  = reg[REG_HP];
word &hf  = reg[REG_HF];

bool powered = true;

typedef enum {
  CPU_RESET,
  CPU_RUNNING,
  CPU_HALT,
  CPU_UNKNOWN
} CPU_global_state_t;
CPU_global_state_t CPU_global_state = CPU_RESET;

void CPU_reset();
void CPU_step();
void CPU_halt();

typedef enum {
  INS_HALT,
  INS_reset,
  INS_IRQ,
  CPU_UNKNOWN
} INS_mask_t;
void INS_halt();
void INS_reset();
void INS_irq(word number);
void INS_jmp(word address);
void INS_ret();
void INS_call(word address);
void INS_push(word regNr);
void INS_pop(word regNr);

int main(int argc, char **argv)
{
  int i;
  for (i = 0; i < argc; i++) fprintf(stderr,"argv[%d] -> %s\n", i, argv[i]);
  fprintf(stderr,"\n");
  
  fprintf(stderr,"reg: %p count: %6lu size:  %6lu Byte\n", reg, reg_len, reg_size );
  fprintf(stderr,"rom: %p count: %6lu size:  %6lu Byte\n", rom, rom_len, rom_size );
  fprintf(stderr,"ram: %p count: %6lu size:  %6lu Byte\n", ram, ram_len, ram_size );
  fprintf(stderr,"irv: %p count: %6lu size:  %6lu Byte\n", irv, irv_len, irv_size );
  fprintf(stderr,"\n");
  fflush(stderr);
  
  
  while( powered ) {
    switch( CPU_global_state ){
      case CPU_RESET:
        log("CPU RESET");
        CPU_reset();
        break;
      case CPU_RUNNING:
        log("CPU RUNNING");
        CPU_step();
        break;
      case CPU_HALT:
        log("CPU HALT");
        CPU_halt();
        break;
      default:
        err("CPU unknown global state");
        powered = false;
        break;
    }
  }
  
  switch( CPU_global_state ){
    case CPU_RESET:
    case CPU_RUNNING:
    case CPU_HALT:
      log("EXIT SUCCESS");
      exit(EXIT_SUCCESS);
      break;
    default:
      err("EXIT FAILURE");
      exit(EXIT_FAILURE);
      break;
  }
  
  return 0;
}

void CPU_reset(){
  for(size_t i=reg_len; i>0; --i) reg[i-1u] = 0u;
  for(size_t i=rom_len; i>0; --i) rom[i-1u] = 0u;
  for(size_t i=ram_len; i>0; --i) ram[i-1u] = 0u;
  for(size_t i=irv_len; i>0; --i) irv[i-1u] = 0u;
  
  sp = ram_len - 1;
  
  INS_call(irv[0]);
  
  CPU_global_state = CPU_RUNNING;
}

void CPU_step(){
  fprintf(stderr,"pc: %04x rt: %04x \nsp: %04x sf: %04x \nhp: %04x hf: %04x\n", pc, ret, sp, sf, hp, hf );
  bool print = false;
  for(word i=0; i<ram_len; ++i){
    word mod = 12;
    if( i%mod==0 && i<=hp && i+mod>hp ) { print = true;  }
    else if( i%mod==0 && i<=sp && i+mod>sp ) { print = true;  }
    else if( i%mod==0) { print = false; }
    
    if( print ){
      
      if( i%mod == 0 ) fprintf(stderr,"\e[0m[%04x]  ", i);
      if( i>sp ) fprintf(stderr,"\e[34m");
      fprintf(stderr,"%04x ", ram[i]);
      if( i%4 == 3 ) fprintf(stderr," ");
      if( i%mod == mod-1 ) fprintf(stderr,"\n");
    }
  }
  fprintf(stderr,"\e[0m\n");
  
  word ins = rom[pc];
  
  
  CPU_global_state = CPU_HALT;
}

void CPU_halt(){
  powered = false;
  CPU_global_state = CPU_UNKNOWN;
}

void INS_halt(){ 
  pc = WORD_MAX;
  CPU_global_state = CPU_HALT; 
}
void INS_reset(){ CPU_global_state = CPU_RESET; }
void INS_irq(word number){ INS_call(irv[number]); }
void INS_jmp(word address){ pc = address; }
void INS_ret(){ pc = ret; }

void INS_call(word address) {
  INS_push( REG_RET );
  ret = pc + 1;
  pc  = address;
}

void INS_push(word regNr){
  ram[sp] = reg[regNr];
  sp -= 1;
  if( sp <= hp ){
    err("Stack collided with heap");
    CPU_global_state = CPU_HALT;
  }
}

void INS_pop(word regNr){
  sp += 1;
  reg[regNr] = ram[sp];
}

