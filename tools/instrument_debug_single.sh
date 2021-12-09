#!/bin/bash



cat $1 | grep 'hook1:' | sed "s/^.*\(hook1:.*\)/\1/g;" > $3
cat $2 | grep 'hook1:' | sed "s/^.*\(hook1:.*\)/\1/g;" > $4

diff -u $3 $4
