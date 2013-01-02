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


int partition(int y[], int f, int l) {
     int up,down,temp;
     int piv = y[f];
     up = f;
     down = l;
     goto partLS;
     do { 
         temp = y[up];
         y[up] = y[down];
         y[down] = temp;
     partLS:
         while (y[up] <= piv && up < l) {
             up++;
         }
         while (y[down] > piv  && down > f ) {
             down--;
         }
     } while (down > up);
     y[f] = y[down];
     y[down] = piv;
     return down;
}

void quicksort(int x[], int first, int last) {
     int pivIndex = 0;
     if(first < last) {
         pivIndex = partition(x,first, last);
         quicksort(x,first,(pivIndex-1));
         quicksort(x,(pivIndex+1),last);
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
	MPI_Status *status;
	
	// Variablen für Merge-Splitting-Sort
	int n;					//Anzahl der zu sortierenden Elemente
	//int nLocal;				//...pro Prozessor
	//int *temp;				//temporäre Ablage
	//int *local;				//lokales Array
	//int *ergebnis;
	double *wtimes;				//Array mit Zeitmessungen
	int i, j;				//Zaehler
	
	printf("\n");
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
	MPI_Comm_size(MPI_COMM_WORLD, &p_world);
	
	printf("Gebe n ein:\n");
	//n auslesen
	//n = 100;	//atoi(argv[1]);
	while (scanf("%i", &n) != 1) while (getchar() != '\n');
	
	int local[2 * nLocal];
	int temp[nLocal];
	double wtimes[p_world*6];
	
	//Allokiere Ergebnis
	//ergebnis = malloc((int)sizeof(int)*p_world*nLocal);
	//ergebnis = malloc(sizeof(int)*p_world*nLocal);
	int ergebnis[p_world*nLocal];

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
//m	//  local = malloc((int)sizeof(int)*2*nLocal);
	//local = malloc(2 * nLocal * sizeof(int));			
	
	//Zur Hälfte mit Zufallszahlen füllen
	for (i=0;i<nLocal;i++) {
		local[i] = rand()  % 100;	//Zahlen 0 bis 99
	}
	
	//temp allokieren
/*	temp = malloc((int)sizeof(int)*nLocal);	*/
	//temp = malloc(nLocal * sizeof(int));					
	
	//Zeitmessungsarray allokieren
	//Pro Prozess pro Runde 6 Messungen
/*m */	//wtimes = malloc(/*(double)*/ sizeof(double) * p_world * 6 );
	
	if (rank_world == 0)
		printf("Jeder Prozess erzeugt sein eigenes Array.\nDas Sortierverfahren wird nun gestartet.\n");
	
	//Wiederhole Runden p_world mal
	for (j = 0; j < p_world; j++)
	{
		//Vorstufe
		printf("P %d: Start Vorstufe\n", rank_world);
		wtimes[j*6] = MPI_Wtime();
		quicksort(local, 0, nLocal-1);
		wtimes[j*6+1] = MPI_Wtime();
		printf("P %d: Ende Vorstufe\nStart ungerader Schritt", rank_world);
		
		//ungerader Schritt
		if ((rank_world+1) % 2 == 0)
		{
			//gerade Prozessornummer
			wtimes[j*6+2] = MPI_Wtime();
			
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(temp, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD, status);
			
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
			MPI_Recv(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD, status);
			
			//füge Arrays zusammen
			for (i = 0; i < nLocal; i++)
			{
				local[nLocal+i] = temp[i];
			}
			
			//sortiere Array
			quicksort(local, 0, nLocal*2-1);
			
			//oberen Teil des Array zum zurücksenden vorbereiten
			for (i = 0; i < nLocal; i++)
			{
				temp[i] = local[nLocal+i];
			}
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
			
			wtimes[j*6+3] = MPI_Wtime();
		}
		
		printf("P %d: Ende ungerader Schritt\nStart gerader Schritt", rank_world);
		//gerader Schritt
		if ((rank_world+1) % 2 == 0 && rank_world != p_world-1)
		{
			//gerade Prozessornummer != Prozessoranzahl-1
			wtimes[j*6+4] = MPI_Wtime();
			
			//erhalte Array
			MPI_Recv(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD, status);
			
			//füge Arrays zusammen
			for (i = 0; i < nLocal; i++)
			{
				local[nLocal+i] = temp[i];
			}
			
			//sortiere Array
			quicksort(local,  0, nLocal*2-1);
			
			//oberen Teil des Array zum zurücksenden vorbereiten
			for (i = 0; i < nLocal; i++)
			{
				temp[i] = local[nLocal+i];
			}
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
			
			wtimes[j*6+5] = MPI_Wtime();
		}
		else if ((rank_world+1) % 2 == 1 && rank_world != 0)
		{
			//ungerade Prozessornummer != 0
			wtimes[j*6+4] = MPI_Wtime();
			
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(temp, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD, status);
			
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
