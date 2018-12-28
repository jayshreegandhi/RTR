#include<stdio.h>

int main(void)
{
	float f;
	float num = 1.7f;

	printf("printing numbers %f to %f :\n",num,(num * 10.0f));

	f = num;
	do
	{
		printf("\t%f\n",f);
		f = f + num;
	}while(f <= (num * 10.0f));

	printf("\n");

	return(0);
}
