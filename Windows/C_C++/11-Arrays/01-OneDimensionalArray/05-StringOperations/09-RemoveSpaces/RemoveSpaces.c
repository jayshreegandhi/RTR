#include<stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
    int MyStrlen(char[]);
    void MyStrcpy(char[], char[]);

    char chArray[MAX_STRING_LENGTH];
    char chArray_SpaceRemoved[MAX_STRING_LENGTH];

    int iStringLength;
    int i,j;

    printf("\n\nEnter a string: \n\n");
    gets_s(chArray, MAX_STRING_LENGTH);

    iStringLength = MyStrlen(chArray);
    j = 0;

    for(i = 0; i< iStringLength; i++)
    {
        if(chArray[i] == ' ')
        {
            continue;
        }
        else
        {
            chArray_SpaceRemoved[j] = chArray[i];
            j++;
        }
    }

    chArray_SpaceRemoved[j] = '\0';


    printf("\n\nString entered by you is:\n");
    printf("%s\n",chArray);

    printf("\n\nString after removal of spaces is : \n\n");
    printf("%s\n", chArray_SpaceRemoved);


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
