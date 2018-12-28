#include<stdio.h>

int main(void)
{
	int age;

	printf("\n");

	printf("Enter age:");
	scanf("%d",&age);

	if(age >= 18)
	{
		printf("entering if-block\n");
		printf("You are eligilble for voting\n");
	}
	else
	{
		printf("entering else-block\n");
		printf("You are not eligilble for voting\n");
	}

	printf("bye\n");

	return(0);
}
