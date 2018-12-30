#include<stdio.h>

int main(void)
{
	int num1;

	printf("\n");

	printf("Enter value for num:");
	scanf("%d",&num1);

	if(num1<0)
	{
		printf("Num = %d is less than 0(negative)\n",num1);
	}

	if((num1 > 0) && (num1 <= 100))
	{
		printf("Num = %d is between 0 and 100\n",num1);
	}

	if((num1 > 100) && (num1 <= 200))
	{
		printf("Num = %d is between 100 and 200\n",num1);
	}

	if((num1 > 200) && (num1 <= 300))
	{
		printf("Num = %d is between 200 and 300\n",num1);
	}

	if((num1 > 300) && (num1 <= 400))
	{
		printf("Num = %d is between 300 and 400\n",num1);
	}

	if((num1 > 400) && (num1 <= 500))
	{
		printf("Num = %d is between 400 and 500\n",num1);
	}

	if(num1 > 500)
	{
		printf("Num = %d is grater than 500\n",num1);
	}

	return(0);
}
