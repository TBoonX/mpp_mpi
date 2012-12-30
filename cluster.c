#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>

#define swap(a,b) {long s_;s_=a;a=b;b=s_;}

/*
 * Vorgehensweise:
 * Jeder Cluster erzeugt n/p Zufallszahlen und speichert sie in einem Array ab.
 * Kommunikation zwischen Slaves findet mit MPI_Send/Recv statt. Besser:  MPI_Bsend
 * Am Ende sammelt Master mit MPI-Gather alle Arrays ein.
 * Zeiterfassung erfolgt mit MPI_Wtime.
*/


int partition(int* A, int n) { // Partition: Wird intern von Quicksort benötigt
  int x = A[0];
  int i = -1;
  int j =  n;
  while (1) {
    do {j--;} while (A[j] > x);
    do {i++;} while (A[i] < x);
    if (i < j) swap(A[i], A[j])
    else       return j+1;
  }
}

void quicksort(int* A, int n) {  // Quicksort: Sequentielle Sortierung
  int q;
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
	MPI_Comm comm_local;		// Lokaler Kommunikator
	
	// Variablen für Merge-Splitting-Sort
	int n;					//Anzahl der zu sortierenden Elemente
	int nLocal;				//...pro Prozessor
	int *temp;				//temporäre Ablage
	int *local;				//lokales Array
	int *ergebnis;
	double *wtimes;				//Array mit Zeitmessungen
	int i, j;				//Zaehler
	
	printf("\n");
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
	MPI_Comm_size(MPI_COMM_WORLD, &p_world);
	
	//n auslesen
	n = atoi(argv[1]);

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
	
	printf("P %d: Initialisierung beendet.\nnLocal = %d\n", rank_world, nLocal);
	
	//Speicher für Zufallszahlen allokieren
	//doppelte Größe von nLocal, da während der Sortierung diese Größe benötigt wird
	local = malloc((int)sizeof(int)*2*nLocal);
	
	//Zur Hälfte mit Zufallszahlen füllen
	for (i=0;i<nLocal;i++) {
		local[i] = rand()  % 100;	//Zahlen 0 bis 99
	}
	
	//Zeitmessungsarray allokieren
	//Pro Prozess pro Runde 6 Messungen
	wtimes = malloc((double)sizeof(double)*p_world*6);
	
	if (rank_world == 0)
		printf("Jeder Prozess erzeugt sein eigenes Array.\nDas Sortierverfahren wird nun gestartet.\n");
	
	//Wiederhole Runden p_world mal
	for (j = 0; j < p_world; j++)
	{
		//Vorstufe
		wtimes[j*6] = MPI_Wtime();
		quicksort(local, nLocal);
		wtimes[j*6+1] = MPI_Wtime();
		
		//ungerader Schritt
		if (rank_world % 2 == 0)
		{
			//gerade Prozessornummer
			wtimes[j*6+2] = MPI_Wtime();
			
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(temp, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			
			//eigenes Array aktualisieren
			for (i = 0; i < nLocal; i++)
			{
				local[i] = temp[i];
			}
			wtimes[j*6+3] = MPI_Wtime();
		}
		else
		{
			//ungerade Prozessornummer
			wtimes[j*6+2] = MPI_Wtime();
			
			//erhalte Array
			MPI_Recv(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
			
			//füge Arrays zusammen
			for (i = 0; i < nLocal; i++)
			{
				local[nLocal+i] = temp[i];
			}
			
			//sortiere Array
			quicksort(local, nLocal*2);
			
			//oberen Teil des Array zum zurücksenden vorbereiten
			for (i = 0; i < nLocal; i++)
			{
				temp[i] = local[nLocal+i];
			}
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
			
			wtimes[j*6+3] = MPI_Wtime();
		}
		
		
		//gerader Schritt
		if (rank_world % 2 == 0 && rank_world != p_world-1)
		{
			//gerade Prozessornummer != Prozessoranzahl-1
			wtimes[j*6+4] = MPI_Wtime();
			
			//erhalte Array
			MPI_Recv(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
			
			//füge Arrays zusammen
			for (i = 0; i < nLocal; i++)
			{
				local[nLocal+i] = temp[i];
			}
			
			//sortiere Array
			quicksort(local, nLocal*2);
			
			//oberen Teil des Array zum zurücksenden vorbereiten
			for (i = 0; i < nLocal; i++)
			{
				temp[i] = local[nLocal+i];
			}
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
			
			wtimes[j*6+5] = MPI_Wtime();
		}
		else if (rank_world % 2 == 1 && rank_world != 0)
		{
			//ungerade Prozessornummer != 0
			wtimes[j*6+4] = MPI_Wtime();
			
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(temp, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			
			//eigenes Array aktualisieren
			for (i = 0; i < nLocal; i++)
			{
				local[i] = temp[i];
			}
			
			wtimes[j*6+5] = MPI_Wtime();
		}
	}
	//Arrays sind sortiert
	
	printf("P %d: ...\nSortierung abgeschlossen!\n\n", rank_world);
	
	//Prozess 0 sammelt alle array
	if (rank_world == 0)
	{
		//Allokiere Ergebnis
		ergebnis = malloc((int)sizeof(int)*p_world*nLocal);
		
		MPI_Gather(local, nLocal, MPI_INT, ergebnis, nLocal, MPI_INT, 0, MPI_COMM_WORLD);
		
		//Ausgabe
		printf("Ergebnis:\n");
		
		for (i = 0; i < nLocal*p_world-1; i++)
		{
			printf(" %d, ", ergebnis[i]);
		}
		printf(" %d\n\n", ergebnis[nLocal*p_world]);
		
		printf("Der gesamte Vorgang dauerte %f\n", wtimes[nLocal*6-1]-wtimes[0]);
	}
	else
	{
		//anderen Prozesse versenden
		MPI_Gather(local, nLocal, MPI_INT, ergebnis, nLocal, MPI_INT, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
