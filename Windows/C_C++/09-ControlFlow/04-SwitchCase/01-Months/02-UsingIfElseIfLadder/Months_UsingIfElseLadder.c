#include<stdio.h>

int main(void)
{
	int month;

	printf("\n");

	printf("Enter month(1 to 12):");
	scanf("%d",&month);

	printf("\n");

	if(month == 1)
	{
		printf("Month number %d is January\n",month);
	}
	else if(month == 2)
	{
		printf("Month number %d is February\n",month);
	}
	else if(month == 3)
	{
		printf("Month number %d is March\n",month);
	}
	else if(month == 4)
	{
		printf("Month number %d is April\n",month);
	}
	else if(month == 5)
	{
		printf("Month number %d is May\n",month);
	}
	else if(month == 6)
	{
		printf("Month number %d is June\n",month);
	}
	else if(month == 7)
	{
		printf("Month number %d is July\n",month);
	}
	else if(month == 8)
	{
		printf("Month number %d is August\n",month);
	}
	else if(month == 9)
	{
		printf("Month number %d is September\n",month);
	}
	else if(month == 10)
	{
		printf("Month number %d is October\n",month);
	}
	else if(month == 11)
	{
		printf("Month number %d is November\n",month);
	}
	else if(month == 12)
	{
		printf("Month number %d is December\n",month);
	}
	else
	{
		printf("Invalid month number %d entered . Please try again\n",month);
	}

	printf("If-else if-else ladder complete\n");
	
	return(0);
}
