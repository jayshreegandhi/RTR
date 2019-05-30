#include<stdio.h>
#include<stdlib.h>

#define SIZE 5

void push(int);
void pop();
int isEmpty();
int isFull();
int getStackSize();

int top = -1;
int stack[SIZE];

int main(void)
{
	push(10);
	push(20);
	push(30);
	push(40);
	push(50);
	push(60);
	push(70);

	//for(int i = getStackSize(); i = 0; i--)
	//{
		//printf("poped element = %d",pop());
	//}

	pop();
	pop();
	pop();
	pop();
	pop();
	pop();
	pop();

	return(0);
}

void push(int data)
{
	if(isFull())
	{
		printf("Stack is full. Overflow . Cannot insert %dth element \n", top+1);
	}
	else
	{
		top = top + 1;
		stack[top] = data;
		printf("Stack[%d] = %d\n",top,data);
	}
}

int isFull(void)
{
	if(top == SIZE)
	{
		return(1);
	}
	else
		return(0);
}

int getStackSize(void)
{
	return top;	
}

void pop(void)
{
	int t = 0;

	if (top == (-1))
	{
		printf("Stack is Empty\n");
	}
	else
	{	
		t = stack[top];
		top = top - 1; 
		printf("poped : %d\n", t);
	}
}
