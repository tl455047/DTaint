# This script is for generate abilist when target trys to include instrumented library 
# and cause undefined reference.

#!/bin/bash

# read error message from build message
if test -f $1 
then
  cat $1 | grep -F 'undefined reference to `dfs$' | sed "s/^.*\(\$.*\'\)/\1/g; s/^./fun:/g; s/.$/=uninstrumented/g;" > $2
  cat $1 | grep -F 'undefined reference to `dfs$' | sed "s/^.*\(\$.*\'\)/\1/g; s/^./fun:/g; s/.$/=discard/g;" >> $2

else
  echo "Usage: ./gen_udr_abilist.sh [error message file] [output_file]"
fi