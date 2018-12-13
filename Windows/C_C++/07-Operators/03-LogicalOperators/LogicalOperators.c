#include<stdio.h>

int main(void)
{
	int a;
	int b;
	int c;
	int result;

	printf("\n\n");
	printf("Enter First Integer : ");
	scanf("%d", &a);

	printf("\n\n");
	printf("Enter Second Integer : ");
	scanf("%d", &b);

	printf("\n\n");
	printf("Enter Third Integer : ");
	scanf("%d", &c);

	printf("\n\n");
	printf("If answer = 0,it is 'FALSE'\n");
	printf("If answer = 1,it is 'TRUE'\n");

	result = (a <= b) && (b != c);
	printf("LOGICAL AND(&&) : answer is TRUE(1) if and only if both conditions are True. The answer is FALSE(0) if any one or both conditions are False\n\n");
	printf("A = %d is less than or equal to B = %d AND B = %d is not equal to C = %d \t Answer = %d\n\n",a,b,b,c,result);

	result = (b >= a) || (a == c);
	printf("LOGICAL OR(||) : answer is FALSE(0) if and only if both conditions are False. The answer is TRUE(1) if any one or both conditions are True\n\n");
	printf("Either B = %d is greater than or equal to A = %d OR A = %d is equal to C = %d \t Answer = %d\n\n",b,a,a,c,result);

	result = !a;
	printf("A = %d and using LOGICAL NOT(!) operator on A gives result = %d\n\n",a,result);

	result = !b;
	printf("B = %d and using LOGICAL NOT(!) operator on B gives result = %d\n\n",b,result);

	result = !c;
	printf("C = %d and using LOGICAL NOT(!) operator on C gives result = %d\n\n",c,result);

	result = (!(a <= b) && !(b != c));
	printf("Using LOGICAL NOT(!) on (a <= b) and also (b != c) and then AND-ing them afterwards gives result = %d\n",result);

	result = !((b >= a) || (a == c));
	printf("Using LOGICAL NOT(!) on entire logical expression (b >= a) || (a == c) gives result = %d\n",result);

	return(0);
}
