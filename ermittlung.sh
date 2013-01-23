echo "n= 20000" > daten.txt
./run.sh -c 2 -i cluster.c -o cluster -v 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 4 -i cluster.c -o cluster -v 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 8 -i cluster.c -o cluster -v 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 10 -i cluster.c -o cluster -v 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 16 -i cluster.c -o cluster -v 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 20 -i cluster.c -o cluster -v 20000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 40000" >> daten.txt

./run.sh -c 2 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 4 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 8 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 10 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 16 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 20 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 80000" >> daten.txt

./run.sh -c 2 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 4 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 8 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 10 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 16 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 20 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt
