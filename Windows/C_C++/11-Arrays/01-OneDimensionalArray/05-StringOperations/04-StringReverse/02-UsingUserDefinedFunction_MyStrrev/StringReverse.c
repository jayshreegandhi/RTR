#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	void MyStrRev(char[], char[]);

	char chArray_Original[MAX_STRING_LENGTH], chArray_Reverse[MAX_STRING_LENGTH];

	printf("\nEnter a string:\n");
	gets_s(chArray_Original,MAX_STRING_LENGTH);

	MyStrRev(chArray_Reverse,chArray_Original);

	printf("\nThe original string entered is :\n");
	printf("%s\n",chArray_Original);

	printf("\nThe reversed string is :\n");
	printf("%s\n",chArray_Reverse);

	return(0);
}

void MyStrRev(char strDest[], char strSource[])
{
	int MyStrLen(char[]);

	int iStringLength =0;
	int i,j,len;

	iStringLength = MyStrLen(strSource);

	len=iStringLength-1;

	for(i = 0, j = len; i < iStringLength, j >= 0; i++,j--)
	{
		strDest[i] = strSource[j];
	}

	strDest[i] = '\0';
}

int MyStrLen(char str[])
{
	int j;
	int strLen = 0;

	for(j =0;j < MAX_STRING_LENGTH; j++)
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

