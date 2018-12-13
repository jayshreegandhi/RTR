#include<stdio.h>

int main(void)
{
	int a, b;
	int p, q;
	char ch_result_1, ch_result_2;
	int i_result_1, i_result_2;

	printf("\n\n");

	a = 7;
	b = 5;
	ch_result_1 = (a > b) ? 'A' : 'B';
	i_result_1 = (a > b) ? a : b;
	printf("Ternary operator answer 1 : %c and %d\n\n",ch_result_1, i_result_1);

	p = 30;
	q = 30;
	ch_result_2 = (p != q) ? 'P' : 'Q';
	i_result_2 = (p != q) ? p : q;
	printf("Ternary operator answer 2 : %c and %d\n\n", ch_result_2, i_result_2);

	printf("\n\n");
	return(0);	
}
