#!/bin/bash


pwd
make clean
make
echo $REPLAY
$REPLAY $ENTRY_FILE_BASE/multiply.entries 2 3 multiply 5 multiply_output.txt

