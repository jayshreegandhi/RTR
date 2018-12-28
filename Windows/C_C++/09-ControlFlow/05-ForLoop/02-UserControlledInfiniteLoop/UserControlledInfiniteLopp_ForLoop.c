#include<stdio.h>
#include<conio.h>

int main(void)
{
	char choice, ch ='\0';

	printf("\n");
	
	printf("Once the infinite loop begins, enter 'Q' or 'q' to quit the infinite for loop\n\n");
	printf("Enter 'Y' or 'y' to initiate user controlled infinite for loop\n");

	choice = getch();
	if(choice == 'Y'  || choice == 'y')
	{
		for(;;)
		{
			printf("In loop\n");
			ch = getch();
			if(ch == 'Q' || ch == 'q')
			{
				break;
			}
		}
	}

	printf("\n");

	printf("exixting user controlled infinite for loop");
	printf("\n");

	return(0);
}
