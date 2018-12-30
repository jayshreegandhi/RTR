#include<stdio.h>

int main(void)
{
	int i,j;

	printf("\n");

	i = 1;
	while(i <= 10)
	{
		printf("i = %d\n\n",i);

		j = 1;
		while(j <= 5)
		{
			printf("\tj = %d\n",j);
			j++;
		}
		i++;
		printf("\n");
	}

	return(0);
}
