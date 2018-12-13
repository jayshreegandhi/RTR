#include<stdio.h>

int main(void)
{
	int a = 5;

	void change_count(void);

	printf("\n");
	printf("A = %d\n\n",a);

	change_count();
	change_count();
	change_count();

	return(0);
}

void change_count(void)
{
	int local_count = 0;

	local_count = local_count + 1;
	printf("Local count = %d\n",local_count);
}
