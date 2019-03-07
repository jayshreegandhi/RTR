#include<stdio.h>
#include<string.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	char chArray_Original[MAX_STRING_LENGTH];

	printf("\nEnter a string:\n");
	gets_s(chArray_Original,MAX_STRING_LENGTH);

	printf("\nThe original string entered is :\n");
	printf("%s\n",chArray_Original);

	printf("\nThe reversed string is :\n");
	printf("%s\n",strrev(chArray_Original));

	return(0);
}
