extern int global_count;

void change_count_one(void)
{
	global_count = global_count + 1;
	printf("Global count = %d\n",global_count);
}
