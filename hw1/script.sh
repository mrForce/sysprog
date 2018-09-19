#!/bin/bash
cd hw1

pwd

make clean
make



echo $REPLAY 
$REPLAY $ENTRY_FILE_BASE/average.entries 1 1 average average_output.txt
$REPLAY $ENTRY_FILE_BASE/powerMod.entries 1 1 powerMod powerMod_output.txt


    


