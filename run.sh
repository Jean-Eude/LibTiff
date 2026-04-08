#!/bin/bash

cd bin
cmake .
make -j4
./main
cd ..