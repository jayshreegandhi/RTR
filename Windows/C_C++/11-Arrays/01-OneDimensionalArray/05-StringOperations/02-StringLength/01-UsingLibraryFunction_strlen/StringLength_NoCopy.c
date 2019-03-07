#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	char chArray[MAX_STRING_LENGTH];
	int iStringLength = 0;

	printf("\nEnter a string:\n");
	gets_s(chArray,MAX_STRING_LENGTH);

	printf("\nString entered by you is:\n");
	printf("%s\n",chArray);

	iStringLength = strlen(chArray);
	printf("\nLength of string is = %d characters!\n", iStringLength);
	
	return(0);
}
