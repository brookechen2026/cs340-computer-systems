#!/bin/sh

leakcheck() {
  rm -f .leaks
  valgrind --trace-children=yes --leak-check=full -q --log-file=.leaks "$@" >/dev/null 2>/dev/null
  [ -e .leaks ] && stat -c %s .leaks || echo 0
  rm .leaks
}

if cpp utf8lib.c | grep -Eq 'printf|puts|putc|\bf?write\b|\bsend\b' 
then
  echo "❌ utflib.c has no printing commands during testing"
  echo "SCORE: 0 / 100"
  exit 0
fi

if [ utf8lib.c -nt tester ]
then
  make clean
  make tester
fi

if [ ! -e tester ]
then
  echo "❌ code compiles"
  echo "SCORE: 0 / 100"
  exit 0
fi

res="$(./tester 2>&1)"
echo "$res"

if [ "${res%SCORE: *}" = "$res" ]
then
  echo "❌ tester crashes"
  echo "SCORE: 0 / 100" 
else
  if [ "$(leakcheck ./tester)" -gt 1 ]
  then
    echo "❌ valgrind passes"
    echo "SCORE MULTIPLIER: 0.75"
  else
    echo "✅ valgrind passes"
    echo "SCORE MULTIPLIER: 1.0"
  fi  
fi

