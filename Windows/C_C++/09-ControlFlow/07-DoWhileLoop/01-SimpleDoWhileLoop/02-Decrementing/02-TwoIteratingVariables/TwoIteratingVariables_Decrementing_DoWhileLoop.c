#include<stdio.h>

int main(void)
{
	int i,j;

	printf("\n");

	printf("printing digits from 10 to 1 and 100 to 10:\n");

	i = 10;
	j = 100;

	do
	{
		printf("\t%d\t%d\n",i,j);
		i--;
		j = j - 10;
	}while(i >= 1, j >= 10);

	printf("\n");

	return(0);
}
