#!/bin/bash
set -e; umask 077
COMP='/usr/bin/gcc'
FLAGS='-std=c++11 -Wall -Wextra -Werror -O0 -g3 -x c++'
LINK='-lstdc++ -fsanitize=address'
LINE_EOF=$(( $(grep -n "^#!EOF\$" vm.cpp | grep -o "^[0-9]*") +1))
if [ "$0" -nt "$0.exe" ]; then
  sed -n -e ''"$LINE_EOF"',$p' "$0" | $COMP $FLAGS -o "$0.exe" - $LINK
fi
set +e; timeout 1 $0.exe "$0" "$@"; STATUS=$?;
exit $STATUS
#!EOF
#line 14 "some.cpp"

#include <stdio.h>

int main( int argC, char** argV ) {
  for (int i = 0; i < argC; i++) 
    printf("argv[%d] -> %s\n", i, argV[i]);
  return 0;
}
