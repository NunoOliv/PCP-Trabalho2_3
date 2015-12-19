void quicksort(int* array,int lo,int hi){

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
    {
      {
        if(lo<j) quicksort(array,lo,j);
        if(i<hi) quicksort(array,i,hi);
      }
    }
}
