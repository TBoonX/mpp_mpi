#!/bin/bash

./run.sh -c 20 -v 20 -i cluster.c -o cluster

for val in 2000000 4000000 8000000
do
	for cpu in 2 4 8 10 16 20
	do
		## echo -e "\n\n\n-> #### val: $val cpu:$cpu ####"
		## echo "val: $val cpu:$cpu `mpirun -np $cpu -hostfile load.txt cluster $val`" 
		mpirun -np $cpu -hostfile load.txt cluster $val
		mpirun -np $cpu -hostfile load.txt cluster $val
		mpirun -np $cpu -hostfile load.txt cluster $val
		mpirun -np $cpu -hostfile load.txt cluster $val
		mpirun -np $cpu -hostfile load.txt cluster $val
		echo ""
	done
done

