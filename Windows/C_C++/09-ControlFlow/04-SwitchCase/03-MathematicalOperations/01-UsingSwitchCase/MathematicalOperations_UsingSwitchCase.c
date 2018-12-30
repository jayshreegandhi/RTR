#include<stdio.h>
#include<conio.h>

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
	printf("'S' or 's' for subtraction\n");
	printf("'M' or 'm' for multiplication\n");
	printf("'D' or 'd' for division\n\n");
	
	printf("Enter option:");
	choice = getch();

	switch(choice)
	{
		case 'A':
		case 'a':
			result = var1 + var2;
			printf("Addition of A = %d and B = %d gives Result %d\n",var1,var2,result);
			break;

		case 'S':
		case 's':
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
			break;

		case 'M':
		case 'm':
			result = var1 * var2;
			printf("Multiplication of A = %d and B = %d gives Result %d\n",var1,var2,result);
			break;

		case 'D':
		case 'd':
			printf("Enter option in character\n");
			printf("'Q' or 'q' or '/' for quotient upon division\n");
			printf("'R' or 'r' or '%%' for remainder upon division\n");

			printf("Enter option:");
			optionDivision = getch();

			switch(optionDivision)
			{
				case 'Q':
				case 'q':
				case '/':
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
					break;

				case 'R':
				case 'r':
				case '%':
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
					break;

				default:
					printf("Invalid character %c entered for division.Please try again\n",optionDivision);
					break;
			}
			break;

			default:
				printf("Invalid character %c entered.Please try again\n",choice);
				break;
	}

	printf("switch-case block complete\n");

	return(0);
}
