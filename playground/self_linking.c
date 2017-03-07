#!/bin/bash
set -e
umask 077
wflags="-Wall -Wextra -Werror"
cflags="-std=gnu11 -Og -ggdb3"
lflags="-fsanitize=address"
if [ "$0" -nt "$0.out" ]; then
  rm -f "$0.o?"
  echo "Compiling..."
  sed -n -e '20,$p' < "$0" | 
    gcc $wflags $cflags -c -o "$0.o1" $lflags -xc -
  objcopy -I binary -O elf64-x86-64 -B i386:x86-64 "$0" "$0.o2"
  gcc -o "$0.out" "$0.o1" "$0.o2" $lflags
fi
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer-3.4
export ASAN_OPTIONS=symbolize=1
$0.out "$0" "$@"
STATUS=$?
exit $STATUS
// bash script EOF

// Multiline Shebang
// src: https://rosettacode.org/wiki/Multiline_shebang
// Can be run as script which compiles its self
// rebuilds only if needed

// src: https://balau82.wordpress.com/2012/02/19/linking-a-binary-blob-with-gcc/
// objcopy -I binary -O <target_format> -B <target_architecture> <binary_file> <object_file>
// objdump -t <object_file>
// readelf -s <object_file>


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>


extern uint8_t _binary___oneshot_link_c_start;
extern uint8_t _binary___oneshot_link_c_end  ;
extern uint8_t _binary___oneshot_link_c_size ;

uint8_t* _binary_start_ptr = &_binary___oneshot_link_c_start;
uint8_t* _binary_end_ptr   = &_binary___oneshot_link_c_end;
size_t   _binary_size      = (size_t)&_binary___oneshot_link_c_size;


int main( int argC, char** argV ) {
    for( int i=0; i<argC; ++i ) {
        printf("arg[%d]: %s \r\n", i, argV[i]);
    }

    printf("Hallo World!\r\n");
    printf("Printing out my own source code...\r\n");

    for( size_t i=0; i<_binary_size; ++i) {
        assert( &_binary_start_ptr[i] < _binary_end_ptr );
        printf("%c", _binary_start_ptr[i]);
    }
    printf("size: %lu\n", _binary_size);

    exit( EXIT_SUCCESS );
}
