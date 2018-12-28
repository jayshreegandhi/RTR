#include<stdio.h>

int main(void)
{
	int num1,num2,i;

	printf("\n");

	printf("Enter an integer value from which iteration must begin:");
	scanf("%d",&num1);

	printf("Enter how many digits do you want to print from %d onwards ? :",num1);
	scanf("%d",&num2);

	printf("printing digits %d to %d :\n",num1,(num1 + num2));

	for(i = num1; i <= (num1 + num2); i++)
	{
		printf("\t%d\n",i);
	}

	printf("\n");

	return(0);
}
