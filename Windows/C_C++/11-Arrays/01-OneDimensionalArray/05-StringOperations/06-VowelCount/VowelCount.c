#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	int MyStrLen(char[]);

	char chArray[MAX_STRING_LENGTH];
	int iStringLength;
	int countA = 0, countE = 0, countI = 0, countO = 0, countU = 0;
	int i;

	printf("\nEnter a string:\n");
	gets_s(chArray,MAX_STRING_LENGTH);

	printf("\nEntered string is:\n");
	printf("%s\n",chArray);

	iStringLength = MyStrLen(chArray);

	for(i=0; i< iStringLength; i++)
	{
		switch(chArray[i])
		{
			case 'A':
			case 'a':
			countA++;
				break;

			case 'E':
			case 'e':
			countE++;
				break;

			case 'I':
			case 'i':
			countI++;
				break;

			case 'O':
			case 'o':
			countO++;
				break;

			case 'U':
			case 'u':
			countU++;
				break;
		}
	}

	printf("\nIn the string entered by you, the vowels and the number of their occurences are:\n");
	printf("'A' has occured = %d times\n",countA);
	printf("'E' has occured = %d times\n",countE);
	printf("'I' has occured = %d times\n",countI);
	printf("'O' has occured = %d times\n",countO);
	printf("'U' has occured = %d times\n",countU);

	return(0);
}

int MyStrLen(char str[])
{
	int j;
	int strLen =0;

	for(j = 0; j < MAX_STRING_LENGTH; j++)
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
