#include<stdio.h>

#define NUM_ELEMENTS 10

int main(void)
{
	int iArray[NUM_ELEMENTS];
	int i,num,sum=0;

	printf("\nEnter integer array elemts for array:\n");
	for(i=0;i<NUM_ELEMENTS;i++)
	{
		scanf("%d",&num);
		iArray[i] = num;
	}

	for(i=0;i<NUM_ELEMENTS;i++)
	{
		sum = sum + iArray[i];
	}

	printf("\nSum of all elements of array : %d\n",sum);

	return(0);
}
