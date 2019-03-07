#include<stdio.h>

#define NUM_ELEMENTS 10

int main(void)
{
	int iArray[NUM_ELEMENTS];
	int i, j, num,count =0;

	printf("\nEnter integer elements for array:\n");
	for(i = 0; i<NUM_ELEMENTS ; i++)
	{
		scanf("%d",&num);

		if(num < 0)
		{
			num = -1 * num;
		}

		iArray[i] = num;
	}

	printf("\nArray elements are:\n");
	for(i = 0; i<NUM_ELEMENTS; i++)
	{
		printf("%d\n",iArray[i]);
	}

	printf("\nPrime numbers amongst the array elements are :\n");
	for(i = 0; i<NUM_ELEMENTS; i++)
	{
		for(j =1; j<= iArray[i]; j++)
		{
			if((iArray[i] % j ) == 0)
			{
				count++;
			}
		}

		if(count == 2)
		{
			printf("%d\n",iArray[i]);
		}

		count = 0;
	}

	return(0);
}
