#include "mpi.h"

#include <stdio.h>

#define swap(a,b) {long s_;s_=a;a=b;b=s_;}

/*
 * Vorgehensweise:
 * Jeder Cluster erzeugt n/p Zufallszahlen und speichert sie in einem Array ab.
 * Kommunikation zwischen Slaves findet mit MPI_Send/Recv statt. Besser:  MPI_Bsend
 * Am Ende sammelt Master mit MPI-Gather alle Arrays ein.
 * Zeiterfassung erfolgt mit MPI_Wtime.
*/


long partition(long* A, long n) { // Partition: Wird intern von Quicksort benötigt
  long x = A[0];
  long i = -1;
  long j =  n;
  while (1) {
    do {j--;} while (A[j] > x);
    do {i++;} while (A[i] < x);
    if (i < j) swap(A[i], A[j])
    else       return j+1;
  }
}

void quicksort(long* A, long n) {  // Quicksort: Sequentielle Sortierung
  long q;
  if (n > 1) {
    q = partition(A, n);
    quicksort(&A[0], q);
    quicksort(&A[q], n-q);
  }
}

int main(int argc, char** argv)
{
	//MPI-Variablen
	int rank_world;			// Rang des Prozesses in MPI_COMM_WORLD
	int rank_local;			// Rang des Prozesses im lokalen Kommunikator
	int p_world;			// Anzahl Prozesse in MPI_COMM_WORLD
	int p_local;			// Anzahl Prozesse im lokalen Kommunikator
	MPI_Comm comm_local;	// Lokaler Kommunikator
	MPI_Status status;
	
	// Variablen für Merge-Splitting-Sort
	int n;					//Anzahl der zu sortierenden Elemente
	int nLocal;				//...pro Prozessor
	int *local;				//lokales Array
	double *wtimes;			//Array mit Zeitmessungen
	int i;					//Zaehler
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
	MPI_Comm_size(MPI_COMM_WORLD, &p_world);
	
	//n auslesen
	n = argv[1];

	if( p_world < 2 || p_world%2 !=0 || n%p_world != 0)	// Gerade Anzahl Prozesse >=2 und n Vielfaches von p?
	{
		if( rank_world == 0)
		{
			printf("Programm bitte mit geradzahliger Prozess-Anzahl >= 2 starten. Bzw. n ist kein Vielfaches von p.\n");
		}
		MPI_Finalize();
		return 0;	
	}
	
	//Anzahl der zu sortierenden Elemente pro Prozessor
	nLocal = n/p_world;
	
	//Speicher für Zufallszahlen allokieren
	//doppelte Größe von nLocal, da während der Sortierung diese Größe benötigt wird
	local = malloc((int*)sizeof(int)*2*nLocal);
	
	//Zur Hälfte mit Zufallszahlen füllen
	for (i=0;i<nLocal;i++) {
		local[i] = rand()  % 100;	//Zahlen 0 bis 99
	}
	
	//Zeitmessungsarray allokieren
	//Pro Prozess pro Runde 6 Messungen
	wtimes = alloc((double*)sizeof(double)*p_world*6);
	
	//Wiederhole Runden p_world mal
	for (i = 0; i < p_world;i++)
	{
		//Vorstufe
		quicksort(local, nLocal);
		
		//Test
		
		
		//gerader Schritt
		
		
		
		//ungerader Schritt
		
		
		
	}

	MPI_Finalize();
	return 0;
}

