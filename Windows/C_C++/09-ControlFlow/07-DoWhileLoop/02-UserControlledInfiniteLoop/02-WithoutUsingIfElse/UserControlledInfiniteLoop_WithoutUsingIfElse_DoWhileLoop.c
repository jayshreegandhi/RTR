#include<stdio.h>

int main(void)
{
	char choice, ch = '\0';

	printf("\n");
	printf("once the infinite loop beging,enter 'Q' or 'q' tp quit the infinite do-while loop");

	do
	{
		do
		{
			printf("\n");
			printf("in loop\n");

			ch = getch();
		}while(ch != 'Q' && ch != 'q');
		printf("\n");
		printf("exitting user controlled infinite do-while loop\n");
		printf("\n");
		printf("do you want to begin user controlled infinite loop again?...(Y/y - yes,any other key - No):");

		choice = getch();
	}while(choice == 'Y' || choice == 'y');

	return(0);
}
