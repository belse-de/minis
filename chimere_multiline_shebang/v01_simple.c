#!/bin/bash
umask 077
COMP='/usr/bin/gcc'
FLAGS='-std=c++11 -Wall -Wextra -Werror -O0 -g3 -x c++'
sed -n -e '11,$p' < "$0" | $COMP $FLAGS -o "$0.$$.exe" -
$0.$$.exe "$0" "$@"
STATUS=$?
rm $0.$$.exe
exit $STATUS
# bash EOF

#include <stdio.h>

int main( int argC, char** argV ) {
  for (int i = 0; i < argC; i++) 
    printf("argv[%d] -> %s\n", i, argV[i]);
  return 0;
}
