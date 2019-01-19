#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	void Addition(void);
	int Subtraction(void);
	void Multiplication(int, int);
	int Division(int, int);

	int resultSubtraction;
	int xMultiplication, yMultiplication;
	int xDivision, yDivision, resultDivision;

	Addition();

	resultSubtraction = Subtraction();
	printf("Subtraction result = %d\n", resultSubtraction);

	printf("\nEnter integer value for 'X' for Multiplication :");
	scanf("%d",&xMultiplication);
	printf("Enter integer value for 'Y' for Multiplication :");
	scanf("%d",&yMultiplication);
	Multiplication(xMultiplication,yMultiplication);

	printf("\nEnter integer value for 'X' for Division :");
	scanf("%d",&xDivision);
	printf("Enter integer value for 'Y' for Division :");
	scanf("%d",&yDivision);
	resultDivision = Division(xDivision,yDivision);
	printf("Division of %d and %d = %d (quotient)\n",xDivision,yDivision,resultDivision);

	return(0);
}

void Addition(void)
{
	int x,y,sum;

	printf("\nEnter integer value for 'X' for Addition :");
	scanf("%d",&x);
	printf("Enter integer value for 'Y' for Addition:");
	scanf("%d",&y);

	sum = x + y;

	printf("Sum of %d and %d = %d\n", x, y, sum);
}

int Subtraction(void)
{
	int x, y, subtraction;

	printf("\nEnter integer value for 'X' for Subtraction :");
	scanf("%d",&x);
	printf("Enter integer value for 'Y' for Subtraction :");
	scanf("%d",&y);

	subtraction = x - y;
	return(subtraction);
}

void Multiplication(int a, int b)
{
	int multiplication;

	multiplication = a * b;

	printf("Multiplication of %d and %d = %d\n",a,b,multiplication);
}

int Division(int a, int b)
{
	int quotient;

	if(a > b)
	{
		quotient = a / b;
	}
	else
	{
		quotient = b / a;
	}

	return(quotient);
}
