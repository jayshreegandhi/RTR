#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	void DisplayInformation(void);
	void FunctionCountry(void);

	DisplayInformation();
	FunctionCountry();

	return(0);
}

void DisplayInformation(void)
{
	void FunctionMy(void);
	void FunctionName(void);
	void FunctionIs(void);
	void FunctionFirstName(void);
	void FunctionMiddleName(void);
	void FunctionSurname(void);
	void FunctionOfAMC(void);

	FunctionMy();
	FunctionName();
	FunctionIs();
	FunctionFirstName();
	FunctionMiddleName();
	FunctionSurname();
	FunctionOfAMC();
}

void FunctionMy(void)
{
	printf("\nMy");
}

void FunctionName(void)
{
	printf("\nName");
}

void FunctionIs(void)
{
	printf("\nIs");
}

void FunctionFirstName(void)
{
	printf("\nJayshree");
}

void FunctionMiddleName(void)
{
	printf("\nGhanshyam");
}

void FunctionSurname(void)
{
	printf("\nGandhi");
}

void FunctionOfAMC(void)
{
	printf("\nOf ASTROMEDICOMP");
}

void FunctionCountry(void)
{
	printf("\nI live in India\n");
}
