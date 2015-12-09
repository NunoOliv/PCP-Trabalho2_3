void maxAndMinArray(int *array, int size, int max, int min) {
	int i;
	
	max = array[0];
	min = array[0];
	
	for (i=1;i<size;i++) {
		if (array[i]>max) max = array[i];
			else {
			if (array[i]<min) min = array[i];
			}
	}
}

void numbersInStep(int *array, int size, int min, int max, int nSize) {
	int i;
	number=0;
	
	for( i=0; i<size; i++) {
		if(min<=array[i]<max) number++;
	}
}