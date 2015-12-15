#include <stdio.h>
//#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>

#include <unistd.h>


#include "QuicksortSeq.h"
#include "AuxFuncs.h"

#define ARRAY_SIZE 10 // 5 milhoes +/- 19MB
#define N_THREADS 1

//Comunication Codes

#define MAX_MIN_MPI 0
#define ARRAY_EXCHANGE_MPI 1



int test (int * array, long double sum) {
//test variables
  long long int sumF=0;
  int lastI=-1;
  int correct=1;
  int i=0;

  lastI=-1;
  while(i<ARRAY_SIZE && correct==1){
  	sumF += array[i];
    if (array[i] < lastI) correct=0;
    lastI=array[i];
  	i++;
  }

  //printf("Sumario depois de ordenaÃ§Ã£o: %lld\n",sumF);
  if (sum!=sumF) correct=0;

  return correct;
}

void printArray(int * array, int size){
  printf("[%d",array[0]);
  int i=1;
  while(i<size){
    printf(", %d",array[i]);
    i++;
  }
  printf("]\n");
}



int main (int argc, char **argv){

  printf("\n\n*******************************************************\n\n");

 //MPI Start
  MPI_Status status;
  MPI_Init (&argc, &argv);
  
  int i=0, j=0;
  long long int sum =0;
  int arraysize;
  int nprocesses;
  int myrank;
  int * array;

 

  MPI_Comm_size (MPI_COMM_WORLD, &nprocesses);
  MPI_Comm_rank (MPI_COMM_WORLD, &myrank);

  arraysize = ( ARRAY_SIZE)/nprocesses;
  //Rand
  srand((unsigned)time(NULL)*myrank*nprocesses);

  //alloc array
  array=(int *)malloc( sizeof(int) * arraysize);

  //printf("A inicializar o array com %d elementos...\n", ARRAY_SIZE);

  while(i<arraysize){
    array[i] = rand() % 100 +1;
    sum += array[i];
    i++;
  }
  printf("myRank =%d",myrank);
  printArray(array, arraysize);
/*
  printf("Concluido!\n");
  printf("Tamanho do array: ");
  double bytes = sizeof(int) * ARRAY_SIZE;
  if(bytes<=1024)  printf("%.3f bytes...\n", (double) bytes);
  if(bytes>1024 && bytes <= 1024*1024)  printf("%.3f Kbytes...\n", (double) bytes/1024);
  if(bytes>1024*1024)  printf("%.3f Mbytes...\n", (double) bytes/(1024*1024));


  printf("A correr o quicksort...\n");
*/
  //double start = omp_get_wtime(); //inicio contagem do tempo


 // high_resolution_clock::time_point t1 = high_resolution_clock::now();

	int maxMin[2];



	//inform max and min to rank0


	if (myrank == 0) {
		sleep(10);
		int * auxArray = (int *) malloc(sizeof(int)*2*nprocesses);
		j=0;
		for(i=1;i<nprocesses;i++) {
			MPI_Recv( &maxMin[0], 2, MPI_INT, i, MAX_MIN_MPI, MPI_COMM_WORLD, &status );
			auxArray[j++] = maxMin[0];
			auxArray[j++] = maxMin[1];
		}
		maxAndMinArray(array, arraysize, auxArray[j], auxArray[j+1]);
		maxAndMinArray(auxArray, nprocesses*2, maxMin[0]+1, maxMin[1]);

		//send new max and min
		for(i=1;i<nprocesses;i++) {
			MPI_Send( &maxMin[0], 2, MPI_INT, i, MAX_MIN_MPI, MPI_COMM_WORLD);
		}
		free(auxArray);

	} else {
		//Find Local Max and Min
		maxAndMinArray(array, arraysize, maxMin[0], maxMin[1]);
		printf("max= %d\nMin=%d\n", maxMin[0], maxMin[1]);
		//Send local Max and Min
		MPI_Send( &maxMin[0], 2, MPI_INT, 0, MAX_MIN_MPI, MPI_COMM_WORLD);
		//Receive global max and min
		MPI_Recv( &maxMin[0], 2, MPI_INT, 0, MAX_MIN_MPI, MPI_COMM_WORLD, &status );

	}
	
	printf("FInished MaxMin");

	//Find Splits
	int * arrayNew;
	int newArraySize=0;
	int number_amount;
	int step = (maxMin[0]-maxMin[1])/nprocesses;
  //Split array

  int ** splitArray = (int ** ) malloc(sizeof(int*)*nprocesses);
  int * splitArrayCounter = (int *) malloc(sizeof(int)*nprocesses);
  int * splitArrayInc =  (int *) malloc(sizeof(int)*nprocesses);

    //ciclo para os contadores
    for(i=0; i<nprocesses; i++) {
      splitArrayCounter[i]=0;
      splitArrayInc[i] = 0;
    }
    for(i=0; i<arraysize; i++) {
      splitArrayCounter[array[i]/step]++; //pode explodir se por causa de arredondamentos der um valor a cima do maximo
    }

    //allocar matriz
    for(i=0; i<nprocesses; i++) {
      splitArray[i] = (int *)malloc(sizeof(int)*splitArrayCounter[i]);
    }

    for(i=0; i<arraysize; i++) {
      j = array[i]/step;
      splitArray[j][splitArrayInc[j]++] = array[i];
    }
    free(array);

    int * probesize =(int *) malloc(sizeof(int)*nprocesses);

	//Send to rank i
	for(i=0; i<nprocesses; i++) {
		//numbersInStep(array,arraysize, maxMin[0], maxMin[1], auxSize );
		int auxSize = splitArrayCounter[i];
		if(myrank == i) {
			newArraySize = auxSize;
      //calcular espaço para alocar matriz
			for(j=0;j<nprocesses; j++) {
				 if (myrank != j) {
					MPI_Probe(j, ARRAY_EXCHANGE_MPI, MPI_COMM_WORLD, &status);
					MPI_Get_count(&status, MPI_INT, &number_amount);
					newArraySize+=number_amount;
          probesize[j]=number_amount;
				 }
			 }
       //enviar elementos para matriz
       int k=0;
      for(j=0;j<nprocesses; j++) {
         MPI_Recv(&arrayNew[k], probesize[j], MPI_INT, j, ARRAY_EXCHANGE_MPI, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         k += number_amount;
       }

       for (j=0;j<auxSize;j++) {
         arrayNew[k] = splitArray[i][j];
       }



		 } else {
       MPI_Send(&splitArray[i][0] , splitArrayCounter[i], MPI_INT, i, MAX_MIN_MPI, MPI_COMM_WORLD);
     }

     free(splitArray[i]);
	}


  quicksort(arrayNew, 0, newArraySize );


 // high_resolution_clock::time_point end = high_resolution_clock::now();

 // auto duration = duration_cast<microseconds>( end - start ).count();
  //double end = omp_get_wtime();  //fim da contagem do tempo

 // cout << "Concluido em " << duration << "microsegundos";
  printf("A iniciar funÃ§Ã£o de teste...\n");

  int r=test(array, sum);

  printf("Concluido!\n");
  if (r==0) printf("Incorreto\n");
  if (r==1) printf("Correto\n");



  printf("\n\n*******************************************************\n\n");
}

//