#include<stdio.h>

int main(void)
{
	int age;

	printf("\n");

	printf("Enter age:");
	scanf("%d",&age);

	if(age >= 18)
	{
		printf("You are eligilble for voting\n");
	}

	return(0);
}
