#include<stdio.h>

int main(void)
{
	int iArrayOne[10];
	int iArrayTwo[10];

	iArrayOne[0] = 1;
	iArrayOne[1] = 2;
	iArrayOne[2] = 3;
	iArrayOne[3] = 4;
	iArrayOne[4] = 5;
	iArrayOne[5] = 6;
	iArrayOne[6] = 7;
	iArrayOne[7] = 8;
	iArrayOne[8] = 9;
	iArrayOne[9] = 10;

	printf("\n");
	printf("Piece-meal (Hard - coded) assignment and display of elements of array 'iArrayOne[]' : \n");
	printf("iArrayOne[0] (1st element) = %d\n", iArrayOne[0]);
	printf("iArrayOne[1] (2nd element) = %d\n", iArrayOne[1]);
	printf("iArrayOne[2] (3rd element) = %d\n", iArrayOne[2]);
	printf("iArrayOne[3] (4th element) = %d\n", iArrayOne[3]);
	printf("iArrayOne[4] (5th element) = %d\n", iArrayOne[4]);
	printf("iArrayOne[5] (6th element) = %d\n", iArrayOne[5]);
	printf("iArrayOne[6] (7th element) = %d\n", iArrayOne[6]);
	printf("iArrayOne[7] (8th element) = %d\n", iArrayOne[7]);
	printf("iArrayOne[8] (9th element) = %d\n", iArrayOne[8]);
	printf("iArrayOne[9] (10th element) = %d\n", iArrayOne[9]);

	printf("\n\n");
	printf("Enter 1st element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[0]);
	printf("Enter 2nd element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[1]);
	printf("Enter 3rd element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[2]);
	printf("Enter 4th element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[3]);
	printf("Enter 5th element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[4]);
	printf("Enter 6th element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[5]);
	printf("Enter 7th element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[6]);
	printf("Enter 8th element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[7]);
	printf("Enter 9th element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[8]);
	printf("Enter 10th element of array 'iArrayTwo[]' : ");
	scanf("%d",&iArrayTwo[9]);

	printf("\n");
	printf("Piece-meal(user input) assignment and display of elements to array 'iArrayTwo[]' : \n\n");
	printf("iArrayTwo[0] (1st element) = %d\n", iArrayTwo[0]);
	printf("iArrayTwo[1] (2nd element) = %d\n", iArrayTwo[1]);
	printf("iArrayTwo[2] (3rd element) = %d\n", iArrayTwo[2]);
	printf("iArrayTwo[3] (4th element) = %d\n", iArrayTwo[3]);
	printf("iArrayTwo[4] (5th element) = %d\n", iArrayTwo[4]);
	printf("iArrayTwo[5] (6th element) = %d\n", iArrayTwo[5]);
	printf("iArrayTwo[6] (7th element) = %d\n", iArrayTwo[6]);
	printf("iArrayTwo[7] (8th element) = %d\n", iArrayTwo[7]);
	printf("iArrayTwo[8] (9th element) = %d\n", iArrayTwo[8]);
	printf("iArrayTwo[9] (10th element) = %d\n", iArrayTwo[9]);	

	return(0);
}
