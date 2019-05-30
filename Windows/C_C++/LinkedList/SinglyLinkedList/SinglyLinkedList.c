#include<stdio.h>
#include<stdlib.h>

struct node
{
	int data;
	struct node *next ;
};

struct node *head = NULL;

struct node * createNode(void);
void insertEnd(void);
void insertHead(void);
void insertBetween(int);
void printList(void);
void deleteNode(int);
void deleteList(void);

int main(void)
{
	int choice;
	int position;
	int n;

	while(1)
	{
		printf("\n\nEnter \n0 - Exit\n1 - Insert at head\n2 - Insert at a position\n3 - Insert at end\n4 - Print\n9 - Delete List\n");
		scanf("%d",&choice);
	
		switch(choice)
		{

			case 0:
				exit(0);
				break;

			case 1:
				insertHead();
				break;

			case 2:
				printf("Enter at which position:\n");
				scanf("%d",&position);
				insertBetween(position);
				break;

			case 3:
				printf("\nHow many node to insert at end :\n");
				scanf("%d",&n);

				for(int i=0;i<n;i++)
				{
					insertEnd();
				}

				break;

			case 4:
				printList();
				break;

			case 9:
				deleteList();
				break;
		}
	}

	return(0);
}

void insertEnd(void)
{
	struct node *temp=NULL;
	struct node *current = NULL;

	
	temp = createNode();

	//create first node 
	if(head == NULL)
	{
		head = temp;
	}
	else
		// insert at end
	{
		current = head;

		while(current->next != NULL)
		{
			current = current -> next;
		}
		current->next = temp;
	}
}

void insertHead(void)
{

	struct node *temp = NULL;
	if(head == NULL)
	{
		head = createNode();
	}
	else
	{
		temp = createNode();

		temp->next = head;
		head = temp;
	}
}

void insertBetween(int position)
{
	struct node *temp = NULL;
	struct node *prev= NULL;
	struct node *current = NULL;
	int k =1;
	if(position == 1)
	{
		insertHead();
		return;
	}
	current = head;
	
	while((current != NULL)&&(k < position))
	{
		k++;
		prev = current;
		current = current->next;
	}
	
	k=k-1;
	if(k != (position-1))
	{
		printf("\nList contains %d elements , Cannot insert at %d position!\n",k,position);
	}
	temp = createNode();
	prev->next = temp;
	temp->next = current;

}

struct node * createNode(void)
{
	int data;
	struct node *temp = NULL;
	temp = (struct node *)malloc(sizeof(struct node));

	printf("Enter data:\n");
	scanf("%d",&data);

	temp->data = data;
	temp->next = NULL;

	return(temp);
}

void printList(void)
{
	struct node *current = head;

	while(current)
	{
		printf("%d ",current->data);
		current = current -> next;
	}

	printf("\n\n");
}

void deleteList(void)
{
	/*struct node *next = NULL;
	struct node *current = NULL;

	if(head == NULL)
	{
		printf("List is already Empty\n");
		exit(0);
	}


	current = head;
	int i =0;

	while(current)
	{
		i++;
		next = current->next;
		printf("deleting %d node\n",i);
		free(current);
		current = next;
	}
	head = NULL;*/

	struct node *temp = NULL;
	int i =0;

	while(head)
	{
		i++;
		printf("deleting %d node\n",i);
		temp = head->next;
		free(head);
		head = temp;
	}
}
