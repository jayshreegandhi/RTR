#include<stdio.h>

int main(void)
{
	int num;

	printf("\n");

	printf("printing even numbers from 0 to 100:\n");

	for(num = 0; num <= 100; num++)
	{
		if(num % 2 != 0)
		{
			continue;
		}
		else
		{
			printf("\t%d\n",num);
		}
	}

	return(0);
}
