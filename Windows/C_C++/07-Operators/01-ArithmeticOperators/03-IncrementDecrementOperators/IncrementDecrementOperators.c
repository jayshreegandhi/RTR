#include<stdio.h>

int main(void)
{
	int a = 5;
	int b = 10;

	printf("\n\n");
	printf("A = %d\n",a);
	printf("A = %d\n",a++);
	printf("A = %d\n",a);
	printf("A = %d\n\n",++a);

	printf("B = %d\n",b);
	printf("B = %d\n",b--);
	printf("B = %d\n",b);
	printf("B = %d\n",--b);

	return(0);
}
