#include<stdio.h>

int main(void)
{
	float f;
	float num = 1.7f;

	printf("\n");

	printf("printing numbers %f to %f :\n", num,(num * 10.0f));

	for(f = num; f <= (num * 10.0f); f = f + num)
	{
		printf("\t%f\n", f);
	}

	printf("\n\n");

	return(0);
}
