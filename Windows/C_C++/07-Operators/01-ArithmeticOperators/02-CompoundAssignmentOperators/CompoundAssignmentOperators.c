#include<stdio.h>

int main(void)
{
	int a;
	int b;
	int x;

	printf("\n\n");
	printf("Enter a number : ");
	scanf("%d", &a);

	printf("\n\n");
	printf("Enter another number : ");
	scanf("%d", &b);

	printf("\n\n");

	x = a;
	a += b;
	printf("Addition of A = %d and B = %d gives %d\n",x,b,a);

	x = a;
	a -= b;
	printf("Subtraction of A = %d and B = %d gives %d\n",x,b,a);

	x = a;
	a *= b;
	printf("Multiplication of A = %d and B = %d gives %d\n",x,b,a);

	x = a;
	a /= b;
	printf("Division of A = %d and B = %d gives quotient %d\n",x,b,a);

	x = a;
	a %= b;
	printf("Division of A = %d and B = %d gives remainder %d\n",x,b,a);

	printf("\n\n");

	return(0);
}
