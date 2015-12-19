#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

//#define ARRAY_SIZE 5*1000*1000 // 5 milhoes +/- 19MB
//#define N_THREADS 1

int * array;


/*int test (long double sum) {
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

  //printf("Sumatório depois de ordenação: %lld\n",sumF);
  if (sum!=sumF) correct=0;

  return correct;
}*/

/*void printArray(){
  printf("[%d",array[0]);
  int i=1;
  while(i<ARRAY_SIZE){
    printf(", %d",array[i]);
    i++;
  }
  printf("]\n");
}*/

void quicksort(int lo,int hi){

  int i=lo,j=hi,h,tID;
  int x=array[(lo+hi)/2];

    do{
        while(array[i]<x) i++;
        while(array[j]>x) j--;

        if(i<=j){
            h=array[i];
            array[i]=array[j];
            array[j]=h;
            i++;
            j--;
        }
    }while(i<=j);

    if(lo<j) quicksort(lo,j);
    if(i<hi) quicksort(i,hi);
}

int main (){

  //printf("\n\n*******************************************************\n\n");

  int i=0,j,k;
  long long int sum =0;
  int arraySize;
  srand(time(NULL));

  for(k=0;k<4;k++){
    if(k==0) arraySize = 5000; //20 KB
    if(k==1) arraySize = 50000; //200 KB
    if(k==2) arraySize = 4000000; //15 MB
    if(k==3) arraySize = 40000000; //152 MB

    for(j=0;j<5;j++){
      srand(time(NULL));
      array=malloc( sizeof(int) * arraySize);

      while(i<arraySize){
        array[i] = rand() % 100 +1;
        i++;
      }


      int clearCache[arraySize];

      while(i<arraySize){
        clearCache[i] = rand() % 100 +1;
        i++;
      }

      double start = omp_get_wtime(); //inicio contagem do tempo
      quicksort(0,arraySize-1);
      double end = omp_get_wtime();  //fim da contagem do tempo

      printf("%d;%f;\n",arraySize,(end-start));
      free(array);
      i=0;
    }
    printf("\n");
  }
}
