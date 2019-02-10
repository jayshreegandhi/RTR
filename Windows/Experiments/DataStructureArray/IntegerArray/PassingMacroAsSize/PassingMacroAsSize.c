#include<stdio.h>
#define SIZE 10

int main(void)
{
	int a[SIZE];

	if(a[SIZE])
	{
		printf("Passing macro as size of array is possible");
	}
	else
	{
		printf("Passing macro as size is not possible");
	}

	return(0);
}
