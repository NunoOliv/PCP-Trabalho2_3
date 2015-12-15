#include <stdio.h>
void maxAndMinArray(int *array, int size, int *maxR, int *minR) {
	int i, max, min;
	printf("1 max =%d\nmin=%d\nsize=%d\n", max, min, size); 
	
	max = array[0];
	min = array[0];
	
	printf("2 max =%d\nmin=%d\n", max, min); 
	for (i=1;i<size-1;i++) {
		printf("2 max =%d\nmin=%d\n", max, min); 
		if (array[i]>max) max = array[i];
			else {
			if (array[i]<min) min = array[i];
			}
	}
	*maxR = max;
	*minR = min;
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
