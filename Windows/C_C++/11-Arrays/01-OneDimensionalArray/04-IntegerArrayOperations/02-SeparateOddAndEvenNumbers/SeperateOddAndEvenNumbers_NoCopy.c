#include<stdio.h>

#define NUM_ELEMENTS 10

int main(void)
{
	int iArray[NUM_ELEMENTS];
	int i,num,sum=0;

	printf("\nEnter integer elemnts for array:\n\n");
	for(i =0;i<NUM_ELEMENTS;i++)
	{
		scanf("%d",&num);
		iArray[i] = num;
	}

	printf("\nEven numbers amongst the array elements are:\n");
	for(i = 0; i< NUM_ELEMENTS;i++)
	{
		if((iArray[i] % 2) == 0)
			printf("%d\n",iArray[i]);
	}

	printf("\nOdd numbers amongst the array elemnts are :\n");
	for(i = 0; i< NUM_ELEMENTS;i++)
	{
		if((iArray[i] % 2) != 0)
			printf("%d\n",iArray[i]);
	}	
	
	return(0);
}
