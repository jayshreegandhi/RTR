#include<stdio.h>

int main(void)
{
	int i,j,k;

	printf("\n");

	i = 0;
	do
	{
		j = 0;
		do
		{
			k = ((i & 0x8) == 0) ^ ((j & 0x8) == 0);

			if(k == 0)
			{
				printf(" ");
			}

			if(k == 1)
			{
				printf("* ");
			}

			j++;

		}while(j < 64);

		printf("\n");
		i++;

	}while(i < 64);

	return(0);
}
