#include<stdio.h>

int main(void)
{
	void PrintBinaryFormOfNumber(unsigned int);

	unsigned int a;
	unsigned int b;
	unsigned int left_shift_a,left_shift_b;
	unsigned int result;

	printf("\n\n");
	printf("Enter a integer : ");
	scanf("%u", &a);

	printf("\n\n");
	printf("Enter another integer : ");
	scanf("%u", &b);

	printf("\n\n");
	printf("By how many bits do you want to shift A = %d to the left ? : ",a);
	scanf("%u", &left_shift_a);

	printf("\n\n");
	printf("By how many bits do you want to shift B = %d to the left ? : ",b);
	scanf("%u", &left_shift_b);

	printf("\n\n");
	result = a << left_shift_a;
	printf("Bitwise LEFT-SHIFT by %d bits of  \nA = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",left_shift_a,a,a,a,result,result,result);
	PrintBinaryFormOfNumber(a);
	PrintBinaryFormOfNumber(result);

	printf("\n\n");
	result = b << left_shift_b;
	printf("Bitwise LEFT-SHIFT by %d bits of  \nB = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",left_shift_b,b,b,b,result,result,result);
	PrintBinaryFormOfNumber(b);
	PrintBinaryFormOfNumber(result);

	return(0);
}

void PrintBinaryFormOfNumber(unsigned int decimal_number)
{
	unsigned int quotient, remainder;
	unsigned int num;
	unsigned int binary_array[8];
	int i;

	for(i = 0;i < 8; i++)
	{
		binary_array[i] = 0;
	}

	printf("the binary form of the decimal integer %d is \t=\t",decimal_number);
	num = decimal_number;
	i=7;

	while(num != 0)
	{
		quotient = num / 2;
		remainder = num % 2;
		binary_array[i] = remainder;
		num = quotient;
		i--;
	}

	for(i = 0;i < 8; i++)
	{
		printf("%u",binary_array[i]);
	}

	printf("\n\n");
}
