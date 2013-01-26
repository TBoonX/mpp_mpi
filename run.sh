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
if [ $# -ne 8 ] ; then	## erzwinge die Angabe aller Startparameter
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
HOSTLISTFILENAME="load.txt"
  
##remove already existing file without warning
touch $HOSTLISTFILENAME         ## create file if not already there
rm $HOSTLISTFILENAME            ## remove file
 
##poke trough simson clients 
for i in 01 02 03 04 05 06 07 08 09 {10..24}
do
    ping -c 1 simson$i  > /dev/null
        if [ $? = 0 ] 
            then
		## Prueft zum einen Erreichbarkeit des SSH-Dienstes und ermittelt mit cat /proc/loadavg auch noch die Auslastung jedes Nodes und grept noch nicht erreichbare Hosts aus dem Ergebnis mittels Regex
                echo "`ping -c 1 simson${i} | grep "64 bytes" | awk ' BEGIN {FS="("} {print $2}' | awk ' BEGIN {FS=")"} {print $1}'` `ssh simson${i} cat /proc/loadavg`" | grep -v -E '141.57.9.[0-9]{2} $' >> $HOSTLISTFILENAME
        fi
done
HOSTNR=`wc -l $HOSTLISTFILENAME | awk '{print $1}'`		## zaehle Anzahl erreichbarer Hosts 

##remove already existing file without warning
touch $HOSTLISTFILENAME.sorted         ## create file if not already there
rm $HOSTLISTFILENAME.sorted            ## remove file

echo "Sortiere ${HOSTNR} Hosts nach Auslastung ..." 		
sort -k 2 $HOSTLISTFILENAME >> $HOSTLISTFILENAME.sorted			
awk '{print $1}' $HOSTLISTFILENAME.sorted > $HOSTLISTFILENAME

echo "Zur Ausfuehrung werde folgenden ${CPUCOUNT} Hosts benutzt, da diese derzeit die geringste Auslastung haben:"
head -n ${CPUCOUNT} ${HOSTLISTFILENAME}.sorted > head.list
awk '{print "Node: " $1 " - Load on this Node:   " $2 " (avg last min)   " $3 " (avg last 5 min)   " $4 " (avg last 15 min) "}' head.list

sleep 1

##3## run program on this hosts
echo "STAGE 3 - run $OUTPUTFILENAME on $CPUCOUNT cpus "
mpirun -np $CPUCOUNT -hostfile $HOSTLISTFILENAME $OUTPUTFILENAME $VRANGE

##cleanup - remove temporary used files
rm $HOSTLISTFILENAME
rm $HOSTLISTFILENAME.sorted
