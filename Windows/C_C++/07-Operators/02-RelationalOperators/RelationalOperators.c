#include<stdio.h>

int main(void)
{
	int a;
	int b;
	int result;

	printf("\n\n");
	printf("enter one integer : ");
	scanf("%d", &a);

	printf("\n\n");
	printf("enter another integer : ");
	scanf("%d", &b);

	printf("\n\n");
	printf("If answer = 0 , it is 'FALSE' \n");
	printf("if answer = 1 , it is 'TRUE' \n");

	result = ( a < b);
	printf("(a < b) A = %d is less than B = %d  \t Answer = %d\n",a,b,result);

	result = ( a > b);
	printf("(a > b) A = %d is greater than B = %d  \t Answer = %d\n",a,b,result);

	result = ( a <= b);
	printf("(a <= b) A = %d is less than or equal to B = %d  \t Answer = %d\n",a,b,result);

	result = ( a >= b);
	printf("(a >= b) A = %d is greater than or equal to B = %d  \t Answer = %d\n",a,b,result);

	result = ( a == b);
	printf("(a == b) A = %d is equal to B = %d  \t Answer = %d\n",a,b,result);

	result = ( a != b);
	printf("(a != b) A = %d is not equal B = %d  \t Answer = %d\n",a,b,result);

	return(0);
}
