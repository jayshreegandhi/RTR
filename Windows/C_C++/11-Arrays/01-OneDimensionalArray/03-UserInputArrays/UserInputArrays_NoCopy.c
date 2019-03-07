#include<stdio.h>

#define INT_ARRAY_NUM_ELEMENTS 5
#define FLOAT_ARRAY_NUM_ELEMENTS 3
#define CHAR_ARRAY_NUM_ELEMENTS	15

int main(void)
{
	int iArray[INT_ARRAY_NUM_ELEMENTS];
	float fArray[FLOAT_ARRAY_NUM_ELEMENTS];
	char cArray[CHAR_ARRAY_NUM_ELEMENTS];
	int i,num;

	printf("\nEnter elements for 'integer' array:\n");
	for(i=0;i<INT_ARRAY_NUM_ELEMENTS;i++)
	{
		scanf("%d",&iArray[i]);
	}

	printf("\nEnter elements for 'floating-point' array:\n");
	for(i=0;i<FLOAT_ARRAY_NUM_ELEMENTS;i++)
	{
		scanf("%f",&fArray[i]);
	}

	printf("\nEnter elements for 'character' array:\n");
	for(i=0;i<CHAR_ARRAY_NUM_ELEMENTS;i++)
	{
		cArray[i] = getch();
		printf("%c\n",cArray[i]);
	}

	printf("\nInteger array elements entered by you:\n");
	for(i=0;i<INT_ARRAY_NUM_ELEMENTS;i++)
	{
		printf("%d\n",iArray[i]);
	}

	printf("\nFloating-point array elements entered by you:\n");
	for(i=0;i<FLOAT_ARRAY_NUM_ELEMENTS;i++)
	{
		printf("%f\n",fArray[i]);
	}

	printf("\nChracter array elements entered by you:\n");
	for(i=0;i<CHAR_ARRAY_NUM_ELEMENTS;i++)
	{
		printf("%c\n",cArray[i]);
	}

	return(0);
}
