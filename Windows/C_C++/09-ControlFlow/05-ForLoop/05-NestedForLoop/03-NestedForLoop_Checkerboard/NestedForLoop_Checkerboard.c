#include<stdio.h>

int main(void)
{
	int i,j,c;

	printf("\n");

	for(i = 0;i < 64; i++)
	{
		for(j = 0; j < 64; j++)
		{
			c = ((i & 0x8) == 0) ^ ((j & 0x8)==0);

			if(c == 0)
			{
				printf(" ");
			}

			if(c == 1)
			{
				printf("* ");
			}
		}
		printf("\n");
	}
	return(0);
}
