#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	void Addition(void);

	Addition();

	return(0);
}

void Addition(void)
{
	int x,y,sum;

	printf("\n enter integer value for 'X' :");
	scanf("%d",&x);

	printf("\n enter integer value for 'Y' :");
	scanf("%d", &y);

	sum = x + y;

	printf("\n sum of %d and %d = %d\n", x, y, sum);
}
