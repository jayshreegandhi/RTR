#include<stdio.h>

int main(void)
{
	void PrintBinaryFormOfNumber(unsigned int);

	unsigned int a;
	unsigned int b;
	unsigned int right_shift_a,right_shift_b;
	unsigned int left_shift_a,left_shift_b;
	unsigned int result;

	printf("\n\n");
	printf("Enter a integer : ");
	scanf("%u", &a);

	printf("\n\n");
	printf("Enter another integer : ");
	scanf("%u", &b);

	printf("\n\n");
	printf("By how many bits do you want to shift A = %d to the right ? : ",a);
	scanf("%u", &right_shift_a);

	printf("\n\n");
	printf("By how many bits do you want to shift B = %d to the right ? : ",b);
	scanf("%u", &right_shift_b);

	printf("\n\n");
	printf("By how many bits do you want to shift A = %d to the left ? : ",a);
	scanf("%u", &left_shift_a);

	printf("\n\n");
	printf("By how many bits do you want to shift B = %d to the left ? : ",b);
	scanf("%u", &left_shift_b);

	printf("\n\n");
	result = a & b;
	printf("Bitwise AND-ing of \nA = %d (Decimal), %o (Octal), %X (Hexadecimal) and \nB = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",a,a,a,b,b,b,result,result,result);
	PrintBinaryFormOfNumber(a);
	PrintBinaryFormOfNumber(b);
	PrintBinaryFormOfNumber(result);

	printf("\n\n");
	result = a | b;
	printf("Bitwise OR-ing of \nA = %d (Decimal), %o (Octal), %X (Hexadecimal) and \nB = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",a,a,a,b,b,b,result,result,result);
	PrintBinaryFormOfNumber(a);
	PrintBinaryFormOfNumber(b);
	PrintBinaryFormOfNumber(result);

	printf("\n\n");
	result = a ^ b;
	printf("Bitwise XOR-ing of \nA = %d (Decimal), %o (Octal), %X (Hexadecimal) and \nB = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",a,a,a,b,b,b,result,result,result);
	PrintBinaryFormOfNumber(a);
	PrintBinaryFormOfNumber(b);
	PrintBinaryFormOfNumber(result);

	printf("\n\n");
	result = ~a;
	printf("Bitwise COMPLEMENT \nA = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",a,a,a,result,result,result);
	PrintBinaryFormOfNumber(a);
	PrintBinaryFormOfNumber(result);

	printf("\n\n");
	result = ~b;
	printf("Bitwise COMPLEMENT \nB = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",b,b,b,result,result,result);
	PrintBinaryFormOfNumber(b);
	PrintBinaryFormOfNumber(result);

	printf("\n\n");
	result = a >> right_shift_a;
	printf("Bitwise RIGHT-SHIFT by %d bits of  \nA = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",right_shift_a,a,a,a,result,result,result);
	PrintBinaryFormOfNumber(a);
	PrintBinaryFormOfNumber(result);

	printf("\n\n");
	result = b >> right_shift_b;
	printf("Bitwise RIGHT-SHIFT by %d bits of  \nB = %d (Decimal), %o (Octal), %X (Hexadecimal) \ngives the result = %d (Decimal), %o (Octal), %X (Hexadecimal)\n\n",right_shift_b,b,b,b,result,result,result);
	PrintBinaryFormOfNumber(b);
	PrintBinaryFormOfNumber(result);

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
