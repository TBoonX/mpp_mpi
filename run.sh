#!/bin/bash

##1##  get filename without extension: (Source: http://stackoverflow.com/questions/965053/extract-filename-and-extension-in-bash)
for fullpath in "$@"
do
    filename="${fullpath##*/}"                      # Strip longest match of */ from start
    dir="${fullpath:0:${#fullpath} - ${#filename}}" # Substring from 0 thru pos of filename
    base="${filename%.[^.]*}"                       # Strip shortest match of . plus at least one non-dot char from end
    ext="${filename:${#base} + 1}"                  # Substring from len of base thru end
    if [[ -z "$base" && -n "$ext" ]]; then          # If we have an extension and no base, it's really the base
        base=".$ext"
        ext=""
    fi

#    echo -e "$fullpath:\n\tdir  = \"$dir\"\n\tbase = \"$base\"\n\text  = \"$ext\""
done


##2##  compile 
echo "compiling $base.$ext ..."
mpicc -Wall -o $base $base.$ext


##3## create hostlist dynamically
echo "creating host list ..."

##simson00-25: 141.57.9.21-46
##check for individual output filename from user given args
HOSTLISTFILENAME="hosts.list"
 
##remove already existing file without warning
touch $HOSTLISTFILENAME 	## create file if not already there
rm $HOSTLISTFILENAME		## remove file
 
##poke trough simson clients 
for i in 01 02 03 04 05 06 07 08 09 {10..24}
do
    ping -c 1 simson$i  > /dev/null
        if [ $? = 0 ] 
            then
                ping -c 1 simson$i | grep "64 bytes" | awk ' BEGIN {FS="("} {print $2}' | awk ' BEGIN {FS=")"} {print $1}' >> $HOSTLISTFILENAME
        fi
done
HOSTNR=`wc -l $HOSTLISTFILENAME | awk '{print $1}'`
echo "${HOSTNR} hosts online"

##4## run program on this hosts
mpirun -np 4 -hostfile $HOSTLISTFILENAME $base	#4 Rechner mit 
