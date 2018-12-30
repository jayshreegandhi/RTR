#include<stdio.h>

int main(void)
{
	int i;

	printf("\n");

	printf("printing digits from 10 to 1:\n");

	i = 10;

	do
	{
		printf("\t%d\n",i);
		i--;
	}while(i >= 1);

	printf("\n");

	return(0);
}
