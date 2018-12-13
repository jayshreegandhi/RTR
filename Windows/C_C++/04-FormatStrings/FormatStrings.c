#include<stdio.h>

int main(void)
{
	printf("\n");
	printf("Hello World !\n\n");

	int a = 13;
	printf("Integer decimal value of 'A' = %d\n",a);
	printf("Integer octal value of 'A' = %o\n",a);
	printf("Integer hexadecimal value of 'A' (Hexadecimal letters in lower case) = %x\n",a);
	printf("Integer hexadecimal value of 'A' (Hexadecimal letters in upper case) = %X\n\n",a);

	char ch = 'P';
	printf("Character ch = %c\n", ch);
	char str[] = "Jayshree Gandhi";
	printf("String str = %s\n\n",str);

	long num = 30121995L;
	printf("Long integer = %ld\n\n",num);

	unsigned int b = 7;
	printf("Unsigned integer 'b' = %u\n\n",b);

	float f_num = 3012.1995f;
	printf("Floating point number with just %%f 'f_num' = %f\n", f_num);
	printf("Floating point number with %%4.2f 'f_num' = %4.2f\n",f_num);
	printf("Floating point number with %%2,5f 'f_num' = %2.5f\n\n",f_num);

	double d_pi = 3.14159265358979323846;
	printf("Double precision floating point number without exponential = %g\n" ,d_pi);
	printf("Double precision floating point number with exponential (lower case) = %e\n" ,d_pi);
	printf("Double precision floating point number with exponential (upper case) = %E\n\n" ,d_pi);
	printf("Double hexadecimal value of 'd_pi' (hexadecimal letter in lower case) = %a\n",d_pi);
	printf("Double hexadecimal value of 'd_pi' (hexadecimal letter in upper case) = %A\n",d_pi);

	return(0);
}
