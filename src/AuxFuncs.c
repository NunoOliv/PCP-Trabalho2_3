#include <stdio.h>
void maxAndMinArray(int *array, int size, int * maxMin) {
	int i, max, min;
	
	max = array[0];
	min = array[0];
	 
	for (i=1;i<size;i++) {
		if (array[i]>max) max = array[i];
			else {
			if (array[i]<min) min = array[i];
			}
			
	}
	
	maxMin[0] = max;
	maxMin[1] = min;
	
}
/*
void numbersInStep(int *array, int size, int min, int max, int nSize) {
	int i;
	number=0;
	
	for( i=0; i<size; i++) {
		if(min<=array[i]<max) number++;
	}
}
*/
