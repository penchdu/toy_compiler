#!/bin/bash



./build/vsc ./test/case2/1.cpp > 2.txt
gcc a0.s -o a0
gcc a1.s -o a1
./a0
./a1
rm a0 a1
 
gcc ./test/test.cpp -o a
./a
rm a

