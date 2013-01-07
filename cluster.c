#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*
 * Vorgehensweise:
 * Zunächst wird ein Array von einem Prozess erzeugt und sortiet, um den Speedup bestimmen zu können.
 * Jeder Cluster erzeugt n/p Zufallszahlen und speichert sie in einem Array ab.
 * Kommunikation zwischen Slaves findet mit MPI_Send/Recv statt. Besser:  MPI_Bsend
 * Am Ende sammelt Master mit MPI-Gather alle Arrays ein.
 * Zeiterfassung erfolgt mit MPI_Wtime.
*/

// siehe http://en.wikibooks.org/wiki/Algorithm_implementation/Sorting/Quicksort#C
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

//int main(int argc, char** argv)
int main(int argc, char* argv[])
{
	//MPI-Variablen
	int rank_world;			// Rang des Prozesses in MPI_COMM_WORLD
	int p_world;			// Anzahl Prozesse in MPI_COMM_WORLD
	MPI_Status *status;
	
	// Variablen für Merge-Splitting-Sort
	int n;					//Anzahl der zu sortierenden Elemente
	int nLocal;				//...pro Prozessor
	int i, j;				//Zaehler
	
	printf("\n");
	
	//Initialisierung der MPI Umgebung
//	MPI_Init(&argc, &argv);
	MPI_Init(0, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
	MPI_Comm_size(MPI_COMM_WORLD, &p_world);
	
	//Festlegen von n
	if (rank_world == 0)
	{
		if(argc == 2) {
			n = atoi(argv[1]);
		} else {
			printf("Gib n ein:\n");
			while (scanf("%i", &n) != 1) while (getchar() != '\n');
		}
	}
		
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	//Anzahl der zu sortierenden Elemente pro Prozessor
	nLocal = n/p_world;

	printf("P %d: n = %d\n", rank_world, n);

	//Test auf korrekte Parameter
	if( p_world < 2 || p_world%2 !=0 || n%p_world != 0)	// Gerade Anzahl Prozesse >=2 und n Vielfaches von p?
	{
		if( rank_world == 0)
		{
			printf("Programm bitte mit geradzahliger Prozess-Anzahl >= 2 starten. Bzw. n ist kein Vielfaches von p.\n");
		}
		MPI_Finalize();
		return 0;	
	}
	
	//Deklarieren der Arrays
	int local[2 * nLocal];			//lokales Array doppelter Größe
	int temp[nLocal*5];			//temporäres Array für Schritte
	double wtimes[p_world*4];		//Zeitmessungen: 4 pro Runde pro Prozessor
	double wtimesinnersort[p_world*2];	//2 Zeitmessungen für sortiervorgang in (un)geraden Schritt
	int ergebnis[p_world*nLocal];		//sortiertes Array
	int singlecore[n];			//von einem Prozess zu sortierendes Array
	long double singlecoretimes[2];		//  Zeit

	status = malloc(sizeof(MPI_Status));
	
	printf("P %d: Initialisierung beendet.\n -> nLocal = %d\n", rank_world, nLocal);
	
	if (rank_world == 0) {
		printf("\nBestimmung von T(1)...\n");
		
		//Array füllen
		for (i=0;i<n;i++) {
			singlecore[i] = rand() % (n*5);		//Zahlen 0 bis n*5
		}
		
		singlecoretimes[0] = MPI_Wtime();
		quicksort(singlecore, 0, n-1);
		singlecoretimes[1] = MPI_Wtime();
		
		printf("   -> T(1) = %LF \n\n", singlecoretimes[1]-singlecoretimes[0]);
	}
	
	//Zur Hälfte mit Zufallszahlen füllen
	srand((unsigned)time(NULL));
	for (i=0;i<nLocal;i++) {
		local[i] = rand()%(n*5);		//Zahlen 0 bis n*5
	}
	
	if (rank_world == 0)
		printf("Jeder Prozess erzeugt sein eigenes Array.\nDas Sortierverfahren wird nun gestartet.\n");
	
	//Wiederhole Runden p_world mal
	for (j = 0; j < p_world; j++)
	{
		//Vorstufe
		printf("P %d: Start Vorstufe\n", rank_world);
		wtimes[j*4] = MPI_Wtime();
		quicksort(local, 0, nLocal-1);
		printf("P %d: Ende Vorstufe\n   Start ungerader Schritt\n", rank_world);
		
		wtimes[j*4+1] = MPI_Wtime();
		
		//ungerader Schritt
		if ((rank_world+1) % 2 == 0)
		{
			//gerade Prozessornummer
			printf("   gerade Prozessornummer\n");
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			printf("   gesendet\n");
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(temp, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD, status);
			
			printf("   ausgetauscht\n");
			
			//eigenes Array aktualisieren
			for (i = 0; i < nLocal; i++)
				local[i] = temp[i];
				
			wtimes[j*4+2] = MPI_Wtime();
			
			printf("   ungeraden Schritt beendet\n");
		}
		else
		{
			//ungerade Prozessornummer
			printf("   ungerade Prozessornummer\n");
			//erhalte Array
			MPI_Recv(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD, status);
			
			printf("   erhalten\n");
			
			//füge Arrays zusammen
			for (i = 0; i < nLocal; i++)
				local[nLocal+i] = temp[i];
			
			//sortiere Array
			wtimesinnersort[0] = MPI_Wtime();
			quicksort(local, 0, nLocal*2-1);
			wtimesinnersort[1] = MPI_Wtime();
			
			printf("   sortiert\n");
			
			//oberen Teil des Array zum zurücksenden vorbereiten
			for (i = 0; i < nLocal; i++)
				temp[i] = local[nLocal+i];
			
			printf("   temp beschrieben\n");
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(temp, nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
			
			printf("   temp gesendet\n");
			
			wtimes[j*4+2] = MPI_Wtime();
		}
		
		printf("P %d: Ende ungerader Schritt\nStart gerader Schritt\n", rank_world);
		//gerader Schritt
		if ((rank_world+1) % 2 == 0 && rank_world != p_world-1)
		{
			//gerade Prozessornummer != Prozessoranzahl-1
			//erhalte Array
			MPI_Recv(temp, nLocal, MPI_INT, rank_world+1, 2, MPI_COMM_WORLD, status);
			
			//füge Arrays zusammen
			for (i = 0; i < nLocal; i++)
				local[nLocal+i] = temp[i];
			
			//sortiere Array
			wtimesinnersort[0] = MPI_Wtime();
			quicksort(local,  0, nLocal*2-1);
			wtimesinnersort[1] = MPI_Wtime();
			
			//oberen Teil des Array zum zurücksenden vorbereiten
			for (i = 0; i < nLocal; i++)
				temp[i] = local[nLocal+i];
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(temp, nLocal, MPI_INT, rank_world+1, 2, MPI_COMM_WORLD);
			
			wtimes[j*4+3] = MPI_Wtime();
		}
		else if ((rank_world+1) % 2 == 1 && rank_world != 0)
		{
			//ungerade Prozessornummer != 0
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 2, MPI_COMM_WORLD);
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(temp, nLocal, MPI_INT, rank_world-1, 2, MPI_COMM_WORLD, status);
			
			//eigenes Array aktualisieren
			for (i = 0; i < nLocal; i++)
				local[i] = temp[i];
			
			wtimes[j*4+3] = MPI_Wtime();
		}
		else
		{
			wtimes[j*4+3] = wtimes[j*4+2];
		}
	}
	//Arrays sind sortiert
	
	printf("P %d: ...\nSortierung abgeschlossen!\n\n", rank_world);
	
	//Prozess 0 sammelt alle array
	if (rank_world == 0)
	{
		MPI_Gather(local, nLocal, MPI_INT, &ergebnis, nLocal, MPI_INT, 0, MPI_COMM_WORLD);
		
		//Ausgabe
		printf("Ergebnis:\n");
		
		for (i = 0; i < nLocal*p_world-2; i++)
			printf(" %d, ", ergebnis[i]);
			
		printf(" %d\n\n", ergebnis[nLocal*p_world-1]);
		
		printf("\n\nDer gesamte Vorgang dauerte %lf Sekunden\n", wtimes[p_world*4-1]-wtimes[0]);
		
		printf("\nSpeedup: S(p) = %lf\n\n", (singlecoretimes[1]-singlecoretimes[0])/(wtimes[p_world*4-1]-wtimes[0]) );
	}
	else
	{
		//anderen Prozesse versenden
		MPI_Gather(local, nLocal, MPI_INT, ergebnis, nLocal, MPI_INT, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
