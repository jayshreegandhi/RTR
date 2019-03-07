#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	char chArray_Original[MAX_STRING_LENGTH];
	char chArray_Copy[MAX_STRING_LENGTH];

	printf("\nEnter a string :\n");
	gets_s(chArray_Original,MAX_STRING_LENGTH);

	strcpy(chArray_Copy,chArray_Original);

	printf("\nThe original string entered by you is:\n");
	printf("%s\n",chArray_Original);

	printf("\nThe copied string is :\n");
	printf("%s\n",chArray_Copy);

	return(0);
}
