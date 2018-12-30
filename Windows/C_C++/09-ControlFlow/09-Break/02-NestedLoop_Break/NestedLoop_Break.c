#include<stdio.h>
#include<conio.h>

int main(void)
{
	int i,j;

	for(i = 1; i <= 20; i++)
	{
		for(j = 1; j <= 20; j++)
		{
			if(j > i)
			{
				break;
			}
			else
			{
				printf("* ");
			}
		}
		printf("\n");
	}

	printf("\n");

	return(0);
}
