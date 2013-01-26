echo "n= 20000" > daten.txt
./run.sh -c 2 -i cluster.c -o cluster -v 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 4 -hostfile load.txt cluster 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 8 -hostfile load.txt cluster 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 10 -hostfile load.txt cluster 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 16 -hostfile load.txt cluster 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 20 -hostfile load.txt cluster 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 40000" >> daten.txt

mpirun -np 2 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 4 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 8 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 10 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 16 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 20 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 80000" >> daten.txt

mpirun -np 2 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 4 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 8 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 10 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 16 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

mpirun -np 20 -hostfile load.txt cluster 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt
