#!/bin/bash
set -e
umask 077
wflags="-Wall -Wextra -Werror"
cflags="-std=gnu11 -Og -ggdb3"
if [ "$0" -nt "$0.out" ]; then
  echo "Compiling..."
  sed -n -e '14,$p' < "$0" | 
    gcc $wflags $cflags -o "$0.out" -xc -
fi
$0.out "$0" "$@"
STATUS=$?
exit $STATUS
// bash script EOF

// Multiline Shebang
// src: https://rosettacode.org/wiki/Multiline_shebang
// Can be run as script which compiles its self
// rebuilds only if needed


#include <stdlib.h>
#include <stdio.h>

int main( int argC, char** argV ) {
    for( int i=0; i<argC; ++i ) {
        printf("arg[%d]: %s \r\n", i, argV[i]);
    }
    printf("Hallo World!\r\n");
    
    exit( EXIT_SUCCESS );
}
