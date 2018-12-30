#include<stdio.h>
#include<conio.h>

int main(void)
{
	int i;
	char ch;

	printf("printing even numbers from 1 to 100 for every user input.Exitting the loop when user enters character 'Q' or 'q'\n");

	printf("enter character 'Q' or 'q' to exit loop\n");

	for(i =1; i <= 100; i++)
	{
		printf("\t%d\n",i);
		ch = getch();

		if(ch == 'Q' || ch == 'q')
		{
			break;
		}
	}

	printf("\n");
	printf("exitting loop\n");

	return(0);
}
