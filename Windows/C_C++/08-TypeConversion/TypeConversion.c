#include<stdio.h>

int main(void)
{
	int i,j;
	char ch_1,ch_2;

	int a,result_int;
	float f, result_float;

	int i_explicit;
	float f_explicit;

	printf("\n\n");

	i=70;
	ch_1 = i;
	printf("I = %d\n",i);
	printf("character 1 (after ch_1 = i) = %c\n\n", ch_1);

	ch_2 = 'Q';
	j= ch_2;
	printf("Character 2 = %c\n", ch_2);
	printf("J (after j = ch_ 2) = %d\n\n", j);

	a = 5;
	f = 7.8f;

	result_float = a + f;
	printf("Integer a = %d and Floating-point number %f added gives floating-point sum =%f\n\n",a,f,result_float);

	result_int = a + f;
	printf("Integer a = %d and Floating-point number %f added gives integer sum =%d\n\n",a,f,result_int); 

	f_explicit = 30.121995f;
	i_explicit = (int)f_explicit;

	printf("Floating point number type casted explicitly = %f\n",f_explicit);
	printf("Resultant integer after explicit type casting of %f = %d\n\n",f_explicit, i_explicit);

	return(0);
}
