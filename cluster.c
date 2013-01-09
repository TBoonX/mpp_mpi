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
 * Jeder Prozess erhebt die zu messenden Daten zu seinen erfassten Zeiten und teilt sie dem Master mit MPI_Reduce() mit.
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

int main(int argc, char* argv[])
{
	int debug = 1;
	
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
//	MPI_Init(&argc, &argv);	//Error: Argumente koennen nicht korrekt durchgereicht werden
	MPI_Init(0, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
	MPI_Comm_size(MPI_COMM_WORLD, &p_world);
	
	//Festlegen von n
	if (rank_world == 0)
	{
		if(argc == 2) {	// wenn n als Startparameter uebergeben wurde
			n = atoi(argv[1]);
		} else {	// sonst von Benutzer erfragen
			printf("Gib n ein:\n");
			while (scanf("%i", &n) != 1) while (getchar() != '\n');
		}
	}
		
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	//Anzahl der zu sortierenden Elemente pro Prozessor
	nLocal = n/p_world;

	if(debug) printf("P %d: n = %d\n", rank_world, n);

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
	double wtimesinnersort[p_world*2];	//2 Zeitmessungen für sortiervorgang in (un)geraden Schritt
	double wtimesphase1[2];		//Zeiten zu Messung der Vorstufe
	double wtimesoverall[2];		//Zeiten zur Bestimmung der gesamten Laufzeit
	int ergebnis[p_world*nLocal];		//sortiertes Array
	int singlecore[n];			//von einem Prozess zu sortierendes Array
	double singlecoretimes[2];		//  Zeit
	double overalltime, overalltime_p;	//Gesamtzeit gesamt
	double speedup, speedup_p;		//Speedup
	double phase1, phase1_p;		//Zeit fuer Phase 1
	double comtime = 0, comtime_p;		//           Zeit zur Kommunikation - Kommunikationsoverhead

	status = malloc(sizeof(MPI_Status));
	
	if (rank_world == 0)
	{
		printf("\nNachfolgend wird ein Array der Groesse %d mit Zufallszahlen nach dem Merge-Splitting Verfahren sortiert.\n", n);
		printf(" -> %d Cluster\n\n", p_world);
	}
	
	if(debug) printf("P %d: Initialisierung beendet.\n", rank_world);
	
	
	printf("\nP %d: Bestimmung von T(1)...\n", rank_world);
		
	//Array füllen
	for (i=0;i<n;i++) {
		singlecore[i] = rand() % (n*5);		//Zahlen 0 bis n*5
	}
	
	singlecoretimes[0] = MPI_Wtime();
	quicksort(singlecore, 0, n-1);
	singlecoretimes[1] = MPI_Wtime();
	
	printf("P %d:    -> T(1) = %f \n\n", rank_world, singlecoretimes[1]-singlecoretimes[0]);
	
	
	
	//Zur Hälfte mit Zufallszahlen füllen
	srand((unsigned)time(NULL));
	for (i=0;i<nLocal;i++) {
		local[i] = rand()%(n*5);		//Zahlen 0 bis n*5
	}
	
	if (rank_world == 0)
		printf("Jeder Prozess erzeugt sein eigenes Array.\nDas Sortierverfahren wird nun gestartet.\n");
		
	//Vorstufe
	if(debug) printf("P %d: Start Vorstufe\n", rank_world);
	wtimesphase1[0] = MPI_Wtime();
	quicksort(local, 0, nLocal-1);
	wtimesphase1[1] = MPI_Wtime();
	if(debug) printf("P %d: Ende Vorstufe\n   Start ungerader Schritt\n", rank_world);
	
	//Wiederhole Runden p_world mal
	for (j = 0; j < p_world; j++)
	{
		wtimesoverall[0] = MPI_Wtime();
		
		//ungerader Schritt
		if ((rank_world+1) % 2 == 0)
		{
			//gerade Prozessornummer
			if(debug) printf("   gerade Prozessornummer\n");
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD);
			if(debug) printf("   gesendet\n");
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(local, nLocal, MPI_INT, rank_world-1, 1, MPI_COMM_WORLD, status);
			
			if(debug) printf("   ausgetauscht\n");
			
			if(debug) printf("   ungeraden Schritt beendet\n");
		}
		else
		{
			//ungerade Prozessornummer
			if(debug) printf("   ungerade Prozessornummer\n");
			//erhalte Array
			MPI_Recv(&local[nLocal], nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD, status);
			
			if(debug) printf("   erhalten\n");
			
			//sortiere Array
			wtimesinnersort[j*2+0] = MPI_Wtime();
			quicksort(local, 0, nLocal*2-1);
			wtimesinnersort[j*2+1] = MPI_Wtime();
			
			if(debug) printf("   sortiert\n");
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(&local[nLocal], nLocal, MPI_INT, rank_world+1, 1, MPI_COMM_WORLD);
		}
		
		if(debug) printf("P %d: Ende ungerader Schritt\nStart gerader Schritt\n", rank_world);
		
		//gerader Schritt
		if ((rank_world+1) % 2 == 0 && rank_world != p_world-1)
		{
			//gerade Prozessornummer != Prozessoranzahl-1
			//erhalte Array
			MPI_Recv(&local[nLocal], nLocal, MPI_INT, rank_world+1, 2, MPI_COMM_WORLD, status);
			
			//sortiere Array
			wtimesinnersort[j*2+0] = MPI_Wtime();
			quicksort(local,  0, nLocal*2-1);
			wtimesinnersort[j*2+1] = MPI_Wtime();
			
			//obere Teil des Arrays wird an Prozessor rank_world+1 gesendet
			MPI_Send(&local[nLocal], nLocal, MPI_INT, rank_world+1, 2, MPI_COMM_WORLD);
		}
		else if ((rank_world+1) % 2 == 1 && rank_world != 0)
		{
			//ungerade Prozessornummer != 0
			//Sendet Array an Prozessor rank_world-1
			MPI_Send(local, nLocal, MPI_INT, rank_world-1, 2, MPI_COMM_WORLD);
			
			//Erhalte oberen Teil des sortieren Arrays
			MPI_Recv(local, nLocal, MPI_INT, rank_world-1, 2, MPI_COMM_WORLD, status);
		}
		else
		{
			//Letzter Prozess fuehrt geraden Schritt nicht aus
			if (rank_world == p_world-1)
			{
				wtimesinnersort[j*2] = wtimesinnersort[j*2+1] = 0;
			}
		}
		wtimesoverall[1] = MPI_Wtime();
	}
	//Arrays sind sortiert
	
	printf("P %d: ...\nSortierung abgeschlossen!\n\n", rank_world);
	
	/*
	if (debug)
	{
		//Ausgabe von wtimes
		printf("\nP%d:  wtimes:\n", rank_world);
		for (i = 0; i < p_world*4; i++)
		{
			printf("P%d: wtimes[%d]=%f\n", rank_world, i, wtimes[i]);
		}
		printf("\n");
	}
	*/
	
	//Jeder Prozess errechnet die zu betrachtenden Zeiten und Werte
	//Gesamtzeit
	overalltime = (wtimesoverall[1]-wtimesoverall[0])+(wtimesphase1[1]-wtimesphase1[0]);
	printf("\nP %d: overalltime=%f\n", rank_world, overalltime);
	
	//Speedup
	speedup = (singlecoretimes[1]-singlecoretimes[0])/overalltime;
	printf("P %d: speedup=%f\n", rank_world, speedup);
	
	//sequentieller Anteil (Sortiervorgang)
	phase1 = wtimesphase1[1]-wtimesphase1[0];
	printf("P %d: phase1=%f\n", rank_world, phase1);
	
	//Zeit die fuer die Kommunikation benötigt wurde
	for (i = 0; i < p_world; i++)
	{
		comtime = comtime+wtimesinnersort[i*2+1]-wtimesinnersort[i*2];
	}
	comtime = overalltime-phase1-comtime;
	printf("P %d: comtime=%f\n", rank_world, comtime);
	
	//Prozess 0 sammelt Werte ein
	
	//Gesamtzeit
	MPI_Reduce(&overalltime, &overalltime_p, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	//Speedup
	MPI_Reduce(&speedup, &speedup_p, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	//Phase 1
	MPI_Reduce(&phase1, &phase1_p, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	//Zeit der Kommunikation
	MPI_Reduce(&comtime, &comtime_p, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	
	
	//Prozess 0 sammelt alle sortierten Zahlen-Arrays ein und gibt die Ergebnisse aus
	if (rank_world == 0)
	{
		MPI_Gather(local, nLocal, MPI_INT, &ergebnis, nLocal, MPI_INT, 0, MPI_COMM_WORLD);
		
		printf("\n\nAlle nachfolgenden Werte sind Durchschnittswerte!\n");
		
		printf("\nDer gesamte Vorgang dauerte %f Sekunden\n", overalltime_p/p_world);
		
		printf("\nSpeedup: S(p) = %f\n", speedup_p/p_world );
		
		printf("\nPhase 1 benoetigte %f Sekunden und somit %f Prozent der Laufzeit.\n", phase1_p/p_world,(phase1_p/p_world)/(overalltime_p/p_world)*100 );
		
		printf("\nDer Kommunikationsoverhead betrung %f Sekunden und somit %f Prozent der Laufzeit.\n", comtime_p/p_world, (comtime_p/p_world)/(overalltime_p/p_world)*100 );
	}
	else
	{
		//anderen Prozesse versenden
		MPI_Gather(local, nLocal, MPI_INT, ergebnis, nLocal, MPI_INT, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
