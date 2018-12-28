#include<stdio.h>

int main(void)
{
	int age;

	printf("\n");
	printf("Enter age:");
	scanf("%d",&age);

	if(age >= 18)
	{
		printf("You are eligible for voting\n");
	}
	
	printf("You are not eligible for voting\n");

	return(0);
}
