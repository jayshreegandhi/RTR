#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	int Addition(void);

	int result;

	result = Addition();

	printf("\nSum = %d\n",result);

	return(0);
}

int Addition(void)
{
	int x,y,sum;

	printf("\n enter integer value for 'X' :");
	scanf("%d",&x);

	printf("\n enter integer value for 'Y' :");
	scanf("%d", &y);

	sum = x + y;

	return(sum);
}
