#include<stdio.h>

int main(void)
{
	int var1,var2;
	int result;

	char choice,optionDivision;

	printf("\n");

	printf("Enter Value for 'A' :");
	scanf("%d",&var1);

	printf("Enter Value for 'B' :");
	scanf("%d",&var2);

	printf("Enter option in character\n");
	printf("'A' or 'a' for addition\n");
	printf("'S' or 's' for subtraction");
	printf("'M' or 'm' for multiplication\n");
	printf("'D' or 'd' for division\n\n");
	
	printf("Enter option:");
	choice = getch();

	if(choice == 'A' || choice == 'a')
	{
			result = var1 + var2;
			printf("Addition of A = %d and B = %d gives Result %d\n",var1,var2,result);
	}
	else if(choice == 'S' || choice == 's')
	{
		if(var1 >= var2)
		{
			result = var1 - var2;
			printf("Subtraction of B = %d from A = %d gives Result %d\n",var2,var1,result);
		}
		else
		{
			result = var2-var1;
			printf("Subtraction of A = %d from B = %d gives Result %d\n",var1,var2,result);	
		}
	}
	else if(choice == 'M' || choice == 'm')
	{
		result = var1 * var2;
		printf("Multiplication of A = %d and B = %d gives Result %d\n",var1,var2,result);
	}
	else if(choice == 'D' || choice == 'd')
	{
		printf("Enter option in character\n");
		printf("'Q' or 'q' or '/' for quotient upon division\n");
		printf("'R' or 'r' or '%%' for remainder upon division\n");

		printf("Enter option:");
		optionDivision = getch();

		if(optionDivision == 'Q' || optionDivision == 'q' || optionDivision == '/')
		{
			if( var1 >= var2)
			{
				result = var1 / var2;
				printf("Divison of A = %d by B = %d gives quotient %d\n",var1,var2,result);
			}
			else
			{
				result = var2 / var1;
				printf("Divison of B = %d by A = %d gives quotient %d\n",var2,var1,result);
			}
		}
		else if(optionDivision == 'R' || optionDivision == 'r' || optionDivision == '%')
		{
			if( var1 >= var2)
			{
				result = var1 % var2;
				printf("Divison of A = %d by B = %d gives remainder %d\n",var1,var2,result);
			}
			else
			{
				result = var2 % var1;
				printf("Divison of B = %d by A = %d gives remainder %d\n",var2,var1,result);
			}
		}
		else
		{
			printf("Invalid character %c entered for division.Please try again\n",optionDivision);
		}
	}
	else
	{
		printf("Invalid character %c entered.Please try again\n",choice);
	}	

	printf("If-else if else ladder complete\n");

	return(0);
}
