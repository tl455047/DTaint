#!/bin/bash

cat $1 | sed "s/dfs.//g; s/DFSanPass.*$//g;" > $3 
cat $2 | sed "s/MemlogPass.*$//g;"  > $4

diff -u $3 $4
