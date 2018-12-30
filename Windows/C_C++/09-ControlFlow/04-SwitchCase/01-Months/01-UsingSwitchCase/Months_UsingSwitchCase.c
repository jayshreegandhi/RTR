#include<stdio.h>

int main(void)
{
	int month;

	printf("\n");

	printf("Enter month(1 to 12):");
	scanf("%d",&month);

	switch(month)
	{
		case 1:
			printf("Month number %d is January\n",month);
			break;

		case 2:
			printf("Month number %d is February\n",month);
			break;

		case 3:
			printf("Month number %d is March\n",month);
			break;

		case 4:
			printf("Month number %d is April\n",month);
			break;

		case 5:
			printf("Month number %d is May\n",month);
			break;

		case 6:
			printf("Month number %d is June\n",month);
			break;

		case 7:
			printf("Month number %d is July\n",month);
			break;

		case 8:
			printf("Month number %d is August\n",month);
			break;

		case 9:
			printf("Month number %d is September\n",month);
			break;

		case 10:
			printf("Month number %d is October\n",month);
			break;

		case 11:
			printf("Month number %d is November\n",month);
			break;

		case 12:
			printf("Month number %d is December\n",month);
			break;

		default:
			printf("Invalid month number %d entered. Please try again\n",month);
			break;
	}

	printf("Switch-case clock complete\n");
	
	return(0);
}
