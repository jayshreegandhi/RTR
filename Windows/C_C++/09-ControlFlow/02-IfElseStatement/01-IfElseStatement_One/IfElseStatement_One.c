#include<stdio.h>

int main(void)
{
	int num1,num2,num3;

	num1 = 9;
	num2 = 30;
	num3 = 30;

	printf("\n");

	if(num1 < num2)
	{
		printf("Entering first if-block\n");
		printf("Number 1 is less than number 2\n");
	}
	else
	{
		printf("Entering first else-block\n");
		printf("Number 1 is not less than number 2\n");	
	}
	printf("First if-else pair done\n");

	printf("\n");
	if(num2 != num3)
	{
		printf("Entering second if-block\n");
		printf("Number 2 is NOT equal to Number 3\n");
	}
	else
	{
		printf("Entering second else-block\n");
		printf("Number 2 is equal to number 3\n");	
	}
	printf("Second if-else pair done\n");

	return(0);
}
