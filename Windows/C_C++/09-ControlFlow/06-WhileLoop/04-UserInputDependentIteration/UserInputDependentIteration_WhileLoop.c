#include<stdio.h>

int main(void)
{
	int num1,num2,i;

	printf("\n");

	printf("enter an integer value from which iteration must begin:");
	scanf("%d",&num1);

	printf("how many digits do you want to print from %d onwards? :",num1);
	scanf("%d",&num2);

	printf("printing digits %d to %d :\n",num1,(num1 + num2));

	i = num1;
	while(i <= (num1 + num2))
	{
		printf("\t%d\n",i);
		i++;
	} 

	printf("\n\n");

	return(0);
}
