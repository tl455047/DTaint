#!/bin/bash

j=0
for i in $1/*
do
    echo $i
    $2 $i > /dev/null 2> log11
    cat log11 | grep 'hook1:' | sed "s/^.*\(hook1:.*\)/\1/g;" > file11
    $3 $i > /dev/null 2> log22
    cat log22 | grep 'hook1:' | sed "s/^.*\(hook1:.*\)/\1/g;" > file22
    DIFF=$(diff file11 file22)
    if [ "$DIFF" == "" ] 
    then
      echo "No Difference"
      
    else
      echo "Bad"
      
    fi
    diff -u file11 file22 > "outfile1/outfile_${j}"
    j=$((j+1))
  
done
#cat $1 | grep 'header: id' | sed "s/^.*\(id.*\'\)/\1/g;" > $3
#cat $2 | grep 'header: id' | sed "s/^.*\(id.*\'\)/\1/g;" > $4

#diff -u $3 $4
