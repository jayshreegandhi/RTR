#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	char chArray_One[MAX_STRING_LENGTH], chArray_Two[MAX_STRING_LENGTH];
	
	printf("\nEnter first string:\n");
	gets_s(chArray_One,MAX_STRING_LENGTH);

	printf("\nEnter second string:\n");
	gets_s(chArray_Two,MAX_STRING_LENGTH);

	printf("********Before Concatenation**************");
	printf("\nThe original first string entered is :\n");
	printf("%s\n",chArray_One);

	printf("\nThe original second string entered is :\n");
	printf("%s\n",chArray_Two);

	strcat(chArray_One,chArray_Two);

	printf("********After Concatenation**************");
	printf("\nchArray_One[] is:\n");
	printf("%s\n",chArray_One );

	printf("\nchArray_Two[] is:\n");
	printf("%s\n",chArray_One);

	return(0);
}
