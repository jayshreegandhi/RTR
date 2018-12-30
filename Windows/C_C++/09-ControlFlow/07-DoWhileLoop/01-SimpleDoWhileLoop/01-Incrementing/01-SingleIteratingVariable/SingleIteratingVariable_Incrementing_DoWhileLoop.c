#include<stdio.h>

int main(void)
{
	int i;

	printf("\n\n");

	printf("printing digits from 1 to 10:\n");

	i = 1;
	do
	{
		printf("\t%d\n", i);
		i++;
	}while(i <= 10);

	printf("\n");

	return(0);
}
