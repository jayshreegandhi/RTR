#include<stdio.h>
#include<conio.h>

int main(void)
{
	char choice, ch ='\0';

	printf("\n");
	printf("once the infinite loop begins, enter 'Q' or 'q' to quit the infinite do while loop\n");
	printf("enter 'Y' or 'y' to initiate user controlled infinite do while loop\n");
	printf("\n");

	choice = getch();

	if(choice == 'Y' || choice == 'y')
	{
		do
		{
			printf("in loop\n");
			ch = getch();

			if(ch == 'Q' || ch == 'q')
			{
				break;
			}
		}while(1);

		printf("\n");
		printf("exitting user controlled infinite do while loop\n");
		printf("\n");
	}
	else
	{
		printf("you must press 'Y' or 'y' to initiate the user controlled do while loop\n");
	}

	return(0);
}
