#include<stdio.h>

int main(void)
{
	int i, j;

	printf("outer loop prints odd numbers between 1 to 10\n");
	printf("inner loop prints even numbers between 1 to 10 for ever odd number printed by outer loop\n");

	for(i = 1; i <= 10; i++)
	{
		if(i % 2 != 0)
		{
			printf("i = %d\n\n",i);

			for(j = 1; j <= 10; j++)
			{
				if(j % 2 == 0)
				{
					printf("\tj = %d\n",j);
				}
				else
				{
					continue;
				}
			}
			printf("\n");
		}
		else
		{
			continue;
		}
	}

	printf("\n");

	return(0);
}
