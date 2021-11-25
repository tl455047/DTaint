# Script for generate target_abilist.txt, all target abilists stored in
# src/abilist/target will be generated when building this project.
# User can also manually generate new target_abilist.txt through this
# script. 
# This script will combine all file in given directory to a single file.
#!/bin/bash
if test -d $1 
then
  find $1 -type f -exec cat {} + > $2
else
  echo "Usage: ./gen_target_abilist.sh [input_dir] [output_file]"
fi