#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
    int MyStrlen(char[]);
    void MyStrcpy(char[], char[]);

    char chArray[MAX_STRING_LENGTH];
    int iStringLength;
    int i;
    int wordCount = 0;
    int spaceCount = 0;

    printf("\n\nEnter a string: \n\n");
    gets_s(chArray, MAX_STRING_LENGTH);

    iStringLength = MyStrlen(chArray);

    for(i = 0; i< iStringLength; i++)
    {
        switch(chArray[i])
        {
            case 32:
            spaceCount++;
            break;

            default:
            break;
        }
    }

    wordCount = spaceCount + 1;

    printf("\n\nString entered by you is:\n");
    printf("%s\n",chArray);

    printf("\n\nNumber of spaces : %d\n",spaceCount);
    printf("\n\nNumber of words : %d\n",wordCount);

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
