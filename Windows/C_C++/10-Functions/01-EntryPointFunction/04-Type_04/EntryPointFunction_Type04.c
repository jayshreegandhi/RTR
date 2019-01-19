#include<stdio.h>

int main(int argc, char *argv[])
{
	int i;

	printf("\nHello World!\n");
	printf("Number of command line arguments = %d\n",argc);

	printf("Command line arguments passed to this parameter are : \n\n");
	for(i = 0; i < argc; i++)
	{
		printf("Command line argument number %d = %s\n",(i + 1),argv[i]);
	}

	return(0);
}
