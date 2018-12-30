#include<stdio.h>
#include<conio.h>

int main(void)
{
	char choice, ch = '\0';

	printf("\n");
	printf("Once the infinite loop begins, enter 'Q' or 'q' to quit the infinite while loop\n");
	printf("enter 'Y' or 'y' to initiate user controlled infinite while loop\n");

	choice = getch();

	if(choice == 'Y' || choice == 'y')
	{
		while(1)
		{
			printf("In loop\n");
			ch = getch();
			if(ch == 'Q' || ch == 'q')
			{
				break;
			}
		}

		printf("\n");
		printf("exitting user controlled infinite while loop");
		printf("\n");
	}
	else
	{
		printf("you must press 'Y' or 'y' to initiate the user controlled infinite while loop\n");
	}

	return(0);
}
