#include<stdio.h>

int main(void)
{
	int i, j, k;

	printf("\n");

	i = 1;

	while(i <= 10)
	{
		printf("i = %d\n\n", i);

		j = 1;
		while(j <= 5)
		{
			printf("\tj = %d\n\n", j);

			k = 1;
			while(k <= 3)
			{
				printf("\t\tk = %d\n",k);
				k++;
			}
			printf("\n\n");
			j++;
		}
		printf("\n\n");
		i++;
	}

	return(0);
}
