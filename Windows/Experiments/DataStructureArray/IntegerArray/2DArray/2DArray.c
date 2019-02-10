#include<stdio.h>

int main(void)
{
	int twoDArray[5][3] = {	{1,2,3},
							{4,5,6},
							{7,8,9},
							{10,11,12},
							{13,14,15}};

	for(int i = 0; i < 5; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			printf("Array element [%d][%d]:",i,j);
			printf("\n");
			printf("Address of element :");
			printf("%d",(int)&twoDArray[i][j]);
			printf("\n\n");
		}
	}
	return(0);
}