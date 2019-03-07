#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	void MyStrCpy(char[], char[]);

	char chArray_Original[MAX_STRING_LENGTH];
	char chArray_Copy[MAX_STRING_LENGTH];

	printf("\nEnter a string :\n");
	gets_s(chArray_Original,MAX_STRING_LENGTH);

	MyStrCpy(chArray_Copy,chArray_Original);

	printf("\nThe original string entered by you is:\n");
	printf("%s\n",chArray_Original);

	printf("\nThe copied string is :\n");
	printf("%s\n",chArray_Copy);

	return(0);
}

void MyStrCpy(char strDest[], char strSource[])
{
	int MyStrLen(char[]);

	int iStringLength = 0;
	int j;

	iStringLength = MyStrLen(strSource);
	for(j = 0; j <iStringLength; j++)
	{
		strDest[j] = strSource[j];
	}

	strDest[j] = '\0';
}

int MyStrLen(char str[])
{
	int strLen = 0;
	int j;

	for(j =0; j<MAX_STRING_LENGTH; j++)
	{
		if(str[j] == '\0')
		{	
			break;
		}
		else
		{
			strLen++;
		}
	}

	return(strLen);
}
