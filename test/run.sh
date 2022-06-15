#!/bin/bash

# acutest
curl https://raw.githubusercontent.com/mity/acutest/master/include/acutest.h > acutest.h

# build test
gcc -std=c99 -o test test.c -I./../src

# run the tests
echo "testing ini..."
./test

# cleanup
rm -f test acutest.h