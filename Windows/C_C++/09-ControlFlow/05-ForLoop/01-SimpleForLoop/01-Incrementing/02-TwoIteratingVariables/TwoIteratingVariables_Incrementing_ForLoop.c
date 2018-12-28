#include<stdio.h>

int main(void)
{
	int i,j;

	printf("Printing digits from 1 to 10 and 10 to 100:\n");

	for(i = 1, j = 10; i <= 10,j <= 100; i++, j = j + 10)
	{
		printf("\t%d\t%d\n",i,j);
	}

	printf("\n");

	return(0);
}
