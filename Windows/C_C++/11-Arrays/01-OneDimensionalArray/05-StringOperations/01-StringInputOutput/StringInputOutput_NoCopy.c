#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	char chArray[MAX_STRING_LENGTH];

	printf("\nEnter a string:\n");
	gets_s(chArray,MAX_STRING_LENGTH);

	printf("String entered by you is:\n");
	printf("%s\n",chArray);

	return(0);
}
