#include<stdio.h>

#define MAX_STRING_LENGTH 512
#define SPACE ' '
#define FULLSTOP '.'
#define COMMA ','
#define EXCLAMATION '!'
#define QUESTION_MARK '?'

int main(void)
{
    int MyStrlen(char[]);
    char MyToUpper(char);

    char chArray[MAX_STRING_LENGTH];
    char chArray_CapitalizedFirstLetter[MAX_STRING_LENGTH];

    int iStringLength;
    int i,j;

    printf("\n\nEnter a string: \n\n");
    gets_s(chArray, MAX_STRING_LENGTH);

    iStringLength = MyStrlen(chArray);
    j = 0;

    for(i = 0; i< iStringLength; i++)
    {
        if(i == 0)
        {
            chArray_CapitalizedFirstLetter[j] = MyToUpper(chArray[i]);
        }
        else if(chArray[i] == SPACE)
        {
            chArray_CapitalizedFirstLetter[j] = chArray[i];
            chArray_CapitalizedFirstLetter[j+1] = MyToUpper(chArray[i+1]);

            j++;
            i++;
        }
        else if((chArray[i] == FULLSTOP || chArray[i] == COMMA || chArray[i] == EXCLAMATION || chArray[i] == QUESTION_MARK) && (chArray[i] != SPACE))
        {
            chArray_CapitalizedFirstLetter[j] = chArray[i];
            chArray_CapitalizedFirstLetter[j + 1] = SPACE;
            chArray_CapitalizedFirstLetter[j + 2] = MyToUpper(chArray[i + 1]);

            j =  j + 2;
            i++;
        }
        else
        {
            chArray_CapitalizedFirstLetter[j] = chArray[i];
        }

        j++;
    }

    chArray_CapitalizedFirstLetter[j] = '\0';


    printf("\n\nString entered by you is:\n");
    printf("%s\n",chArray);

    printf("\n\nString after capitalizing first word : \n\n");
    printf("%s\n", chArray_CapitalizedFirstLetter);


    return(0);
}

int MyStrlen(char str[])
{
    int j;
    int stringLength = 0;

    for(j =0; j < MAX_STRING_LENGTH; j++)
    {
        if(str[j] == '\0')
        {
            break;
        }
        else
        {
            stringLength++;
        }
    }

    return(stringLength);
}

char MyToUpper(char ch)
{
    int num;
    int c;

    num = 'a' - 'A';

    if((int)ch >= 97 && (int)ch <= 122)
    {
        c = (int)ch - num;
        return((char)c);
    }
    else
    {
        return(ch);
    }

}
