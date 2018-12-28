#include<stdio.h>
#include<conio.h>

// A to Z - 65 to 90
#define CHAR_UPPER_CASE_ALPHABET_START	65
#define CHAR_UPPER_CASE_ALPHABET_END	90

// a to z - 97 to 122
#define CHAR_LOWER_CASE_ALPHABET_START	97
#define CHAR_LOWER_CASE_ALPHABET_END	122

//0 to 9 - 48 to 57
#define CHAR_DIGIT_START	48
#define CHAR_DIGIT_END		57

int main(void)
{
	char ch;
	int chValue;

	printf("\n");

	printf("Enter Character:");
	ch = getch();

	switch(ch)
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
			printf("Character \'%c\' entered by you is a VOWEL\n",ch);
			break;

		default:
			chValue = (int)ch;

			if((chValue >= CHAR_UPPER_CASE_ALPHABET_START && chValue <= CHAR_UPPER_CASE_ALPHABET_END) || (chValue >= CHAR_LOWER_CASE_ALPHABET_START && chValue <= CHAR_LOWER_CASE_ALPHABET_END))
			{
				printf("Character \'%c\' entered by you is a CONSONANT\n",ch);
			}
			else if(chValue >= CHAR_DIGIT_START && chValue <= CHAR_DIGIT_END)
			{
				printf("Character \'%c\' entered by you is a DIGIT\n",ch);	
			}
			else
			{
				printf("Character \'%c\' entered by you is a SPECIAL CHARATER\n",ch);
			}
			break;	
	}

	printf("Switch case block complete\n");

	return(0);
}
