#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	int MyStrLen(char[]);

	char chArray[MAX_STRING_LENGTH];
	int iStringLength = 0;

	printf("\nEnter a string:\n");
	gets_s(chArray,MAX_STRING_LENGTH);

	printf("\nString entered by you is:\n");
	printf("%s\n",chArray);

	iStringLength = MyStrLen(chArray);
	printf("\nLength of string is = %d characters!\n", iStringLength);
	
	return(0);
}

int MyStrLen(char str[])
{
	int j;
	int stringLen = 0;

	for(j=0;j<MAX_STRING_LENGTH;j++)
	{
		if(str[j] == '\0')
		{
			break;
		}
		else
		{
			stringLen++;
		}
	}

	return(stringLen);
}
