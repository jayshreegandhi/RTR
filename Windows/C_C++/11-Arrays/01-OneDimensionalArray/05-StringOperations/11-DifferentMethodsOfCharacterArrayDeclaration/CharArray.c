#include<stdio.h>

int main(void)
{
    char chArray_01[] = {'A','S','T','R','O','M','E','D','I','C','O','M','P','\0'};
    char chArray_02[] = {'W','E','L','C','O','M','E','S','\0'};
    char chArray_03[] = {'Y','O','U','\0'};
    char chArray_04[] = {'T','O','\0'};
    char chArray_05[] = "REAL TIME RENDERING BATCH OF 2018-2019";

    char chArray_withoutNullTerminator[] = {'H','e','l','l','o'};

    printf("\n\n");

    printf("Size of chArray_01 : %zd\n\n", sizeof(chArray_01));
    printf("Size of chArray_02 : %zd\n\n", sizeof(chArray_02));
    printf("Size of chArray_03 : %zd\n\n", sizeof(chArray_03));
    printf("Size of chArray_04 : %zd\n\n", sizeof(chArray_04));
    printf("Size of chArray_05 : %zd\n\n", sizeof(chArray_05));

    printf("\n\n");

    printf("The Strings are :\n\n");
    printf("chArray_01 : %s\n\n", chArray_01);
    printf("chArray_02 : %s\n\n", chArray_02);
    printf("chArray_03 : %s\n\n", chArray_03);
    printf("chArray_04 : %s\n\n", chArray_04);
    printf("chArray_05 : %s\n\n", chArray_05);

    printf("\n\n");
    printf("Size of chArray_withoutNullTerminator : %zd\n\n", sizeof(chArray_withoutNullTerminator));
    printf("\nchArray_withoutNullTerminator : %s\n\n", chArray_withoutNullTerminator);
    return(0);
}
