#include<stdio.h>

int main(void)
{
	int num1;

	printf("\n");

	num1 = 5;
	if(num1)
	{
		printf("if-block 1 : number exits and has value = %d\n",num1);
	}

	num1 = -5;
	if(num1)
	{
		printf("if-block 2 : number exits and has value = %d\n",num1);
	}

	num1 = 0;
	if(num1)
	{
		printf("if-block 3 : number exits and has value = %d\n",num1);
	}

	printf("all 3 if-statements are done\n");
	
	return(0);
}
