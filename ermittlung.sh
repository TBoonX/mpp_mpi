echo "n= 800000" > daten.txt
./run.sh -c 4 -i cluster.c -o cluster -v 800000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 8 -i cluster.c -o cluster -v 800000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 10 -i cluster.c -o cluster -v 800000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 16 -i cluster.c -o cluster -v 800000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 600000" >> daten.txt

./run.sh -c 6 -i cluster.c -o cluster -v 600000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 12 -i cluster.c -o cluster -v 600000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 60000" >> daten.txt
./run.sh -c 4 -i cluster.c -o cluster -v 60000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 6 -i cluster.c -o cluster -v 60000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 8 -i cluster.c -o cluster -v 60000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 10 -i cluster.c -o cluster -v 60000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 12 -i cluster.c -o cluster -v 60000  | grep "\->" >> daten.txt
echo "" >> daten.txt

./run.sh -c 16 -i cluster.c -o cluster -v 60000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 30000" >> daten.txt
./run.sh -c 10 -i cluster.c -o cluster -v 30000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 40000" >> daten.txt
./run.sh -c 10 -i cluster.c -o cluster -v 40000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 50000" >> daten.txt
./run.sh -c 10 -i cluster.c -o cluster -v 50000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 60000" >> daten.txt
./run.sh -c 10 -i cluster.c -o cluster -v 60000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 70000" >> daten.txt
./run.sh -c 10 -i cluster.c -o cluster -v 70000  | grep "\->" >> daten.txt
echo "" >> daten.txt

echo "n= 80000" >> daten.txt
./run.sh -c 10 -i cluster.c -o cluster -v 80000  | grep "\->" >> daten.txt
echo "" >> daten.txt
