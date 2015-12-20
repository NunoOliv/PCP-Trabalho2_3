#include <stdio.h>
//#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>

#include <unistd.h>


#include "QuicksortSeq.h"
#include "AuxFuncs.h"

#define ARRAY_SIZE 4000000		// 5 milhoes +/- 19MB
#define N_THREADS 1

//Comunication Codes

#define MAX_MIN_MPI 0
#define ARRAY_EXCHANGE_MPI 1



int
test (int *array, long double sum)
{
//test variables
  long long int sumF = 0;
  int lastI = -1;
  int correct = 1;
  int i = 0;

  lastI = -1;
  while (i < ARRAY_SIZE && correct == 1)
    {
      sumF += array[i];
      if (array[i] < lastI)
	correct = 0;
      lastI = array[i];
      i++;
    }

  //printf("Sumario depois de ordenaÃ§Ã£o: %lld\n",sumF);
  if (sum != sumF)
    correct = 0;

  return correct;
}

void
printArray (int *array, int size)
{
  if (size == 0)
    {
      //printf ("[]\n");
      return;
    }
  //printf ("[%d", array[0]);
  int i = 1;
  while (i < size)
    {
      //printf (", %d", array[i]);
      i++;
    }
  //printf ("]\n");
}

void
printMatrix (int **matrix, int *size, int sizeA)
{

  //printf ("[");
  //printf ("[%d", matrix[0][0]);
  int i = 1, j = 1;
  while (i < size[0])
    {
      //printf (", %d", matrix[0][i]);
      i++;
    }
  //printf ("]\n");
  i = 1;
  while (i < sizeA)
    {
      //printf (",\n ");

      //printf ("[%d", matrix[i][0]);
      j = 1;
      while (j < size[i])
	{
	  //printf (", %d", matrix[i][j]);
	  j++;
	}
      //printf ("]\n");

      i++;
    }
  //printf ("]\n");
}



int
main (int argc, char **argv)
{

  //printf("\n\n*******************************************************\n\n");



  //MPI Start
  MPI_Status status;
  MPI_Init (&argc, &argv);

  int nprocesses;
  int myrank;

  MPI_Comm_size (MPI_COMM_WORLD, &nprocesses);
  MPI_Comm_rank (MPI_COMM_WORLD, &myrank);

  int i = 0, j = 0;
  long long int sum = 0;
  int arraysize;


  int *array;



  arraysize = (ARRAY_SIZE) / nprocesses;
  //Rand
  srand ((unsigned) time (NULL) * myrank * nprocesses);

  //alloc array
  array = (int *) malloc (sizeof (int) * arraysize);

  //printf("A inicializar o array com %d elementos...\n", ARRAY_SIZE);

  while (i < arraysize)
    {
      array[i] = rand () % 100 + 1;
      sum += array[i];
      i++;
    }
  //printf ("myRank =%d, nProcesses = %d", myrank, nprocesses);
  //printArray (array, arraysize);
/*
  //printf("Concluido!\n");
  //printf("Tamanho do array: ");
  double bytes = sizeof(int) * ARRAY_SIZE;
  if(bytes<=1024)  //printf("%.3f bytes...\n", (double) bytes);
  if(bytes>1024 && bytes <= 1024*1024)  //printf("%.3f Kbytes...\n", (double) bytes/1024);
  if(bytes>1024*1024)  //printf("%.3f Mbytes...\n", (double) bytes/(1024*1024));


  //printf("A correr o quicksort...\n");
*/
  //double start = omp_get_wtime(); //inicio contagem do tempo


  // high_resolution_clock::time_point t1 = high_resolution_clock::now();

    double start, stop;

  MPI_Barrier(MPI_COMM_WORLD);
  start = MPI_Wtime();

  int maxMin[2] = { 0, 0 };

  //inform max and min to rank0


  if (myrank == 0)
    {
      //sleep(10);

      int *auxArray = (int *) malloc (sizeof (int) * 2 * nprocesses);

      j = 0;
      for (i = 1; i < nprocesses; i++)
	{
	  //printf("mASTER try to Recebeu from %d\n", i);
	  MPI_Recv (maxMin, 2, MPI_INT, i, MAX_MIN_MPI, MPI_COMM_WORLD,
		    &status);
	  //printf("mASTER Recebeu from %d, [%d,%d]\n", i, maxMin[0], maxMin[1]);
	  auxArray[j++] = maxMin[0];
	  auxArray[j++] = maxMin[1];
	}
      //printf("mASTER REcebeu\n");
      maxAndMinArray (array, arraysize, maxMin);
      //printf("mASTER maxmin own array\n");
      auxArray[j++] = maxMin[0];
      auxArray[j++] = maxMin[1];
      maxAndMinArray (auxArray, nprocesses * 2, maxMin);
      //printf("mASTER maxmin own maxminarray\n");
      maxMin[0] += nprocesses;	//batota para garantir que nao explode
      //send new max and min
      for (i = 1; i < nprocesses; i++)
	{
	  MPI_Send (maxMin, 2, MPI_INT, i, MAX_MIN_MPI, MPI_COMM_WORLD);
	}
      //printf("mASTER sent new maxint\n");
      free (auxArray);

    }
  else
    {
      //Find Local Max and Min
      maxAndMinArray (array, arraysize, maxMin);
      //printf("%d - Please max= %d\nMin=%d\n", myrank,maxMin[0], maxMin[1]);
      //Send local Max and Min
      MPI_Send (maxMin, 2, MPI_INT, 0, MAX_MIN_MPI, MPI_COMM_WORLD);
      //printf("Slave Enviou\n");
      //Receive global max and min
      MPI_Recv (maxMin, 2, MPI_INT, 0, MAX_MIN_MPI, MPI_COMM_WORLD, &status);
      //printf("Slave REcebeu\n");

    }



  //Find Splits
  int *arrayNew;
  int newArraySize = 0;
  int number_amount;
  int step = (maxMin[0] - maxMin[1]) / nprocesses;

  //if (myrank == 0)
    //printf ("%d - FInished MaxMin [%d,%d], Step = %d \n", myrank, maxMin[0],maxMin[1], step);
	    


  //Split array

  int **splitArray = (int **) malloc (sizeof (int *) * nprocesses);
  int *splitArrayCounter = (int *) malloc (sizeof (int) * nprocesses);
  int *splitArrayInc = (int *) malloc (sizeof (int) * nprocesses);



  //ciclo para os contadores
  for (i = 0; i < nprocesses; i++)
    {
      splitArrayCounter[i] = 0;
      splitArrayInc[i] = 0;
    }

  for (i = 0; i < arraysize; i++)
    {
      splitArrayCounter[(array[i] - maxMin[1]) / step]++;	//pode explodir se por causa de arredondamentos der um valor a cima do maximo
    }


  //allocar matriz
  for (i = 0; i < nprocesses; i++)
    {
      splitArray[i] = (int *) malloc (sizeof (int) * splitArrayCounter[i]);
    }


  for (i = 0; i < arraysize; i++)
    {
      j = (array[i] - maxMin[1]) / step;
      splitArray[j][splitArrayInc[j]++] = array[i];
    }

  free (array);
  //printf ("%d - Imprimir Split \n", myrank);
  //printMatrix(splitArray,splitArrayCounter, nprocesses);


  int *probesize = (int *) malloc (sizeof (int) * nprocesses);
  int **bufferArray = (int **) malloc (sizeof (int) * nprocesses);

  //Send to rank i
  for (i = 0; i < nprocesses; i++)
    {
      //Barreira Em Principio nao e preciso
      //MPI_Barrier (MPI_COMM_WORLD);
      //numbersInStep(array,arraysize, maxMin[0], maxMin[1], auxSize );
      int auxSize = splitArrayCounter[i];
      if (myrank == i)
	{
	  //printf ("%d - è a minha Vez \n", myrank);
	  newArraySize = auxSize;
	  //calcular espaço para alocar matriz
	  for (j = 0; j < nprocesses; j++)
	    {
	      if (myrank != j)
		{
		  MPI_Probe (j, ARRAY_EXCHANGE_MPI, MPI_COMM_WORLD, &status);
		  MPI_Get_count (&status, MPI_INT, &number_amount);
		  newArraySize += number_amount;
		  probesize[j] = number_amount;
		  bufferArray[j] =
		    (int *) malloc (sizeof (int) * number_amount);
		  //printf ("%d -Dei Probe ao %d e recebi %d \n", myrank, j,number_amount);
			  
		  MPI_Recv (bufferArray[j], probesize[j], MPI_INT, j,
			    ARRAY_EXCHANGE_MPI, MPI_COMM_WORLD,
			    MPI_STATUS_IGNORE);
		  //printf ("%d -Recebi do %d - ", myrank, j);
		  //printArray (bufferArray[j], probesize[j]);
		}
	    }


	}
      else
	{
	  //printf("%d - Pronto a enviar para %d\n", myrank,i);
	  MPI_Send (&splitArray[i][0], splitArrayCounter[i], MPI_INT, i,
		    ARRAY_EXCHANGE_MPI, MPI_COMM_WORLD);
	  //printf ("%d - Enviei para %d\n", myrank, i);
	  free (splitArray[i]);
	}


    }




  arrayNew = (int *) malloc (sizeof (int) * newArraySize);
  int k = 0;

//Até aqui esta bem
  //printf ("%d - Cont %d - SplitMy", myrank, splitArrayCounter[myrank]);
  //printArray (splitArray[myrank], splitArrayCounter[myrank]);
  //printMatrix(splitArray,splitArrayCounter,nprocesses);

  for (j = 0; j < splitArrayCounter[myrank]; j++)
    {

      arrayNew[k] = splitArray[myrank][j];
      k++;
    }

  free (splitArray[myrank]);
  free (splitArray);
  free (splitArrayCounter);
  //printf ("%d - Mae estou aqui\n", myrank);
  for (j = 0; j < nprocesses; j++)
    {
      if (myrank != j)
	{
	  for (i = 0; i < probesize[j]; i++)
	    {

	      arrayNew[k] = bufferArray[j][i];
	      k++;
	    }
	}
    }



  //printf ("%d - Comecei o quicksort \n", myrank);
  //printf ("%d - Pre Sort", myrank);
  //printArray (arrayNew, newArraySize);
  quicksort (arrayNew, 0, newArraySize);
  
  MPI_Barrier(MPI_COMM_WORLD);
  stop = MPI_Wtime();

  if (myrank==0) printf("%f Please\n", (stop-start));
  MPI_Barrier(MPI_COMM_WORLD);
  //printf ("%d - Acabei o quicksort \n", myrank);
  //printArray (arrayNew, newArraySize);
  //printArray(arrayNew,newArraySize);

/*

  // high_resolution_clock::time_point end = high_resolution_clock::now();

  // auto duration = duration_cast<microseconds>( end - start ).count();
  //double end = omp_get_wtime();  //fim da contagem do tempo

  // cout << "Concluido em " << duration << "microsegundos";
  //printf ("A iniciar funÃ§Ã£o de teste...\n");

  int r = test (array, sum);

  //printf ("Concluido!\n");
  if (r == 0)
    //printf ("Incorreto\n");
  if (r == 1)
    //printf ("Correto\n");

*/

  //printf ("\n\n*******************************************************\n\n");
  //MPI_Finalize ();
}

//
