#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	int i;

	printf("\nHello World!\n");

	printf("Number of command line arguments = %d\n\n",argc);

	printf("Command line arguments passed to program are :\n\n");
	for(i = 0; i < argc; i++)
	{
		printf("Command line argument number %d = %s\n", (i+1),argv[i]);
	}
	printf("\n");

	printf("First 20 enviornment variables passed to this program are :\n");
	for(i = 0; i < 20; i++)
	{
		printf("Command line argument number %d = %s\n", (i + 1), envp[i]);
	}

	printf("\n");

	return(0);
}
