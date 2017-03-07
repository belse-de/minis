#!/bin/bash
umask 077
COMP='/usr/bin/gcc'
FLAGS='-std=c++11 -Wall -Wextra -Werror -O0 -g3 -x c++'
if [ "$0" -nt "$0.exe" ]; then
  sed -n -e '12,$p' < "$0" | $COMP $FLAGS -o "$0.exe" -
fi
$0.exe "$0" "$@"
STATUS=$?
exit $STATUS
# bash EOF

#include <stdio.h>

int main( int argC, char** argV ) {
  for (int i = 0; i < argC; i++) 
    printf("argv[%d] -> %s\n", i, argV[i]);
  return 0;
}
