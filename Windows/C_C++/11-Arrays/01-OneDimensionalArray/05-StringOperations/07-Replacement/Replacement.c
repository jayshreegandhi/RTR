#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	int MyStrLen(char[]);
	void MyStrCpy(char[],char[]);

	char chArray_Original[MAX_STRING_LENGTH];
	char chArray_VowelsReplaced[MAX_STRING_LENGTH];

	int iStringLength;
	int i;

	printf("\nEnter a string :\n");
	gets_s(chArray_Original,MAX_STRING_LENGTH);

	MyStrCpy(chArray_VowelsReplaced,chArray_Original);

	iStringLength = MyStrLen(chArray_VowelsReplaced);

	for(i = 0; i<iStringLength; i++)
	{
		switch(chArray_VowelsReplaced[i])
		{
			case 'A':
			case 'a':
			case 'E':
			case 'e':
			case 'I':
			case 'i':
			case 'O':
			case 'o':
			case 'U':
			case 'u':
				chArray_VowelsReplaced[i] = '*';
				break;
			default:
				break;
		}
	}

	printf("\nString entered by you is:\n");
	printf("%s\n",chArray_Original);

	printf("\nString after replacementof vowels by * is :\n");
	printf("%s\n",chArray_VowelsReplaced);

	return(0);
}

int MyStrLen(char str[])
{
	int j;
	int strLen = 0;

	for(j=0;j<MAX_STRING_LENGTH; j++)
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

void MyStrCpy(char strDest[], char strSource[])
{
	int MyStrLen(char[]);

	int iStringLength = 0;
	int j;

	iStringLength = MyStrLen(strSource);

	for(j=0;j<MAX_STRING_LENGTH;j++)
		strDest[j] = strSource[j];

	strDest[j] = '\0';
}
