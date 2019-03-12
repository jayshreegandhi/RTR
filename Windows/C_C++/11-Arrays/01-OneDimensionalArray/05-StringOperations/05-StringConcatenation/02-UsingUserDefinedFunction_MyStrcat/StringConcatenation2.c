#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	void MyStrCat(char[],char[]);

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

	MyStrCat(chArray_One,chArray_Two);

	printf("********After Concatenation**************");
	printf("\nchArray_One[] is:\n");
	printf("%s\n",chArray_One );

	printf("\nchArray_Two[] is:\n");
	printf("%s\n",chArray_One);

	return(0);
}

void MyStrCat(char strDest[], char strSource[])
{
	int MyStrLen(char[]);

	int iStringLengthSource = 0;
	int iStringLengthDest = 0;
	int i,j;

	iStringLengthSource = MyStrLen(strSource);
	iStringLengthDest = MyStrLen(strDest);

	for(i = iStringLengthDest,j =0; j<iStringLengthSource; i++,j++)
	{
		strDest[i] = strSource[i];
	}

	strDest[i] = '\0';
}

int MyStrLen(char str[])
{
	int j;
	int strLen = 0;

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
