#!/bin/bash     

for (( i=0; i < 25; i++ )) # clear screen
do
 echo
done

g++ -g -DSC_DBG_UNIX -ILinux -I.. linux.cpp ../ServoComposer.cpp Linux/unixCompile.cpp -lrt -o ServoComposerLinux
