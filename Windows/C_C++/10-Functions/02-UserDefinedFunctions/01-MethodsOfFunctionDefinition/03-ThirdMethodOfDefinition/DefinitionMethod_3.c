#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	void Addition(int , int);

	int x , y;

	printf("\n enter integer value for 'X' :");
	scanf("%d",&x);

	printf("\n enter integer value for 'Y' :");
	scanf("%d", &y);
	
	Addition(x,y);

	return(0);
}

void Addition(int a, int b)
{
	int sum;
	sum = a + b;

	printf("\nSum of %d and %d = %d\n",a,b,sum);
}
