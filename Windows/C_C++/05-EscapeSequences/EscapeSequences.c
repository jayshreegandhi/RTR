#include<stdio.h>

int main(void)
{
	printf("\n\n");
	printf("Going on to next line using \\n escape sequence\n\n");
	printf("Demonstrating \t horizontal \t tab \t using \t \\t escape sequence \n\n");
	printf("\"This is a double quoted output\" done using \\\" \\\" escape sequence\n\n");
	printf("\'This is a single quoted output\' done using \\\' \\\' escape sequence\n\n");
	printf("Backspace turned to backspace\b using escape sequence \\b\n\n");

	printf("\r Demonstrating carriage return using \\r escape sequence\n");
	printf("Demonstrating \r carriage return using \\r escape sequence\n");
	printf("Demonstrating carriage \r return using \\r escape sequence\n\n");

	printf("Demonstrating \x41 using \\xhh escape sequence\n\n");
	printf("Demonstrating \102 using \\ooo escape sequence\n\n");

	return(0);
}
