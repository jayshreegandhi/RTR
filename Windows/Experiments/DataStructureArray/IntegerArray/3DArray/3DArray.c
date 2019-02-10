#include<stdio.h>

int main(void)
{
	int twoDArray[5][3][2] = {
		{ {1,2}, {3,4}, {5,6} },
		{ {7,8}, {9,10}, {11,12} },
		{ {13,14}, {15,16}, {17,18} },
		{ {19,20}, {21,22}, {23,24} },
		{ {25,26}, {27,28}, {29,30} }
	};

	for(int i = 0; i < 5; i++)
	{
		for(int j =0; j < 3; j++)
		{
			for(int k = 0; k < 2; k++)
			{
				printf("Array element [%d][%d][%d]:",i,j,k);
				printf("\n");
				printf("Address of element :");
				printf("%d",(int)&twoDArray[i][j][k]);
				printf("\n\n");
			}
		}
	}
	return(0);
}
