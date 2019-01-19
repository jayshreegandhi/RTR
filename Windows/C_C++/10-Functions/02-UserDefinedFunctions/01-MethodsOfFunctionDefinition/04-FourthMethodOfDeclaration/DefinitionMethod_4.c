#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	int Addition(int , int);

	int x , y ,result;

	printf("\n enter integer value for 'X' :");
	scanf("%d",&x);

	printf("\n enter integer value for 'Y' :");
	scanf("%d", &y);
	
	result =  Addition(x, y);

	printf("\nSum of %d and %d = %d\n",x,y,result);

	return(0);
}

int Addition(int a, int b)
{
	int sum;
	sum = a + b;

	return(sum);
}
