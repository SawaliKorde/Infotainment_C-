#!/bin/bash

#to find inside file - awk/error/{print} logfile.txt
# use find and grep

echo "Enter string_input to search in filesystem"
read string_input


find / -type f -exec grep "$string_input" {} + 2>/dev/null




