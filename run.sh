#!/bin/bash

##0##  process script args:
CPUCOUNT=0
VRANGE=0
INPUTFILENAME="cluster.c"
OUTPUTFILENAME="cluster"


function usage {
        echo "Usage: $0 -c CPUCOUNT -v VALUERANGE -i INPUTFILE -o OUTPUTFILE"
        exit 1;
}

##8 params required
if [ $# -ne 8 ] ; then
        usage;
fi

##process args
while getopts c:hi:o:v: opt
do
        case "$opt" in
                c) CPUCOUNT=$OPTARG;;
                h) usage;;
                i) INPUTFILENAME=$OPTARG;;
                o) OUTPUTFILENAME=$OPTARG;;
                v) VRANGE=$OPTARG;;
                \?) usage;;
        esac
done

echo "CPUCOUNT: $CPUCOUNT"
echo "VRANGE:   $VRANGE"
echo "INPUT:    $INPUTFILENAME"
echo "OUT:      $OUTPUTFILENAME"


##1##  compile 
echo "STAGE 1 - compiling $INPUTFILENAME ..."
mpicc -Wall -o $OUTPUTFILENAME $INPUTFILENAME


##2## create hostlist dynamically
echo "STAGE 2 - creating host list ..."

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
                ping -c 1 simson$i | grep "64 bytes" | awk ' BEGIN {FS="("} {print $2}' | awk ' BEGIN {FS=")"} {print $1}' | grep -v 141.57.9.25 | grep -v 141.57.9.29 | grep -v 141.57.9.45 >> $HOSTLISTFILENAME
        fi
done
HOSTNR=`wc -l $HOSTLISTFILENAME | awk '{print $1}'`
echo "${HOSTNR} hosts online"

##3## run program on this hosts
echo "STAGE 3 - run $OUTPUTFILENAME on $CPUCOUNT cpus "
mpirun -np $CPUCOUNT -hostfile $HOSTLISTFILENAME $OUTPUTFILENAME $VRANGE
